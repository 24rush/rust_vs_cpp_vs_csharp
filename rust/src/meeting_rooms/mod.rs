use std::{
    cmp::Reverse,
    collections::{BinaryHeap, HashMap, HashSet},
    sync::{Arc, Condvar, Mutex, RwLock},
    thread,
};

use chrono::{DateTime, Duration, Local};

use self::{
    intervaltree::{Data, IntervalTree},
    types::{DateTimeSlot, MeetingRoom, MeetingRoomBooking},
};

mod intervaltree;
mod tests;
mod types;

type IntervalType = DateTime<Local>;
type IntervalPayload = String;

#[derive(PartialEq)]
enum CleanupContextStopReason {
    None = 0,
    Stop,
    Restart,
}

struct CleanupContext {
    wake_reason: Mutex<CleanupContextStopReason>,
    cv_wake_cleanup_thread: Condvar,
}

impl CleanupContext {
    pub fn new() -> Self {
        Self {
            wake_reason: Mutex::new(CleanupContextStopReason::None),
            cv_wake_cleanup_thread: Condvar::new(),
        }
    }
}

#[derive(Default)]
struct MeetingRoomSchedulerData {
    i_tree: IntervalTree<IntervalType, IntervalPayload>,
    meeting_rooms: HashMap<IntervalPayload, MeetingRoom>,
    end_times: BinaryHeap<Reverse<IntervalType>>,
}

struct MeetingRoomScheduler {
    cleanup_thread: Option<thread::JoinHandle<()>>,
    cleanup_context: Arc<CleanupContext>,
    sync_data: Arc<RwLock<MeetingRoomSchedulerData>>,
}

impl Drop for MeetingRoomScheduler {
    fn drop(&mut self) {
        self.notify_cleanup_thread(CleanupContextStopReason::Stop);
        let _ = self.cleanup_thread.take().unwrap().join();
    }
}

impl MeetingRoomScheduler {
    pub fn new() -> Self {
        let mut this = Self {
            cleanup_thread: None,
            sync_data: Arc::new(RwLock::new(Default::default())),
            cleanup_context: Arc::new(CleanupContext::new()),
        };

        let c_ctx = this.cleanup_context.clone();
        let s_data = this.sync_data.clone();

        this.cleanup_thread = Some(thread::spawn(move || {
            MeetingRoomScheduler::run_cleanup(s_data, c_ctx);
        }));

        this
    }

    pub fn register_room(&self, m: &MeetingRoom) {
        self.sync_data
            .write()
            .unwrap()
            .meeting_rooms
            .insert(m.get_name(), m.clone());
    }

    pub fn request_room(&self, ts: &DateTimeSlot) -> Option<MeetingRoomBooking> {
        let avail_room: Option<MeetingRoom> = 'avail: {
            let read_data = self.sync_data.read().unwrap();

            let rooms_booked_in_interval =
                MeetingRoomScheduler::find_conflicting_rooms(&read_data, ts);

            for room in &read_data.meeting_rooms {
                if !rooms_booked_in_interval.contains(room.0) {
                    break 'avail Some(room.1.clone());
                }
            }

            None
        };

        if let Some(room) = avail_room {
            let restart_cleanup;

            {
                let mut write_data = self.sync_data.write().unwrap();

                write_data.i_tree.insert(Data::new(
                    ts.get_start_time(),
                    ts.get_end_time(),
                    &room.get_name(),
                ));

                restart_cleanup = write_data.end_times.len() > 0
                    && ts.get_end_time() < write_data.end_times.peek().unwrap().0;

                write_data.end_times.push(Reverse(ts.get_end_time()));
            }

            if restart_cleanup {
                self.notify_cleanup_thread(CleanupContextStopReason::Restart);
            }

            return Some(MeetingRoomBooking::new(&room, ts));
        }

        None
    }

    pub fn request_explicit_room(
        &self,
        room_name: &String,
        ts: &DateTimeSlot,
    ) -> Option<MeetingRoomBooking> {
        let meeting_room = self
            .sync_data
            .read()
            .unwrap()
            .meeting_rooms
            .get(room_name)
            .cloned();

        // Meeting room with name room_name does not exit
        if meeting_room.is_none() {
            return None;
        }

        let mr = meeting_room.unwrap();

        let can_book =
            !MeetingRoomScheduler::find_conflicting_rooms(&self.sync_data.read().unwrap(), ts)
                .contains(&mr.get_name());

        if can_book {
            let mut write_data = self.sync_data.write().unwrap();

            write_data.i_tree.insert(Data::new(
                ts.get_start_time(),
                ts.get_end_time(),
                &mr.get_name(),
            ));
            write_data.end_times.push(Reverse(ts.get_end_time()));

            return Some(MeetingRoomBooking::new(&mr, ts));
        }

        None
    }

    pub fn cancel_booking(&self, booking: &MeetingRoomBooking) {
        self.sync_data.write().unwrap().i_tree.remove(&Data::new(
            booking.time_slot.get_start_time(),
            booking.time_slot.get_end_time(),
            &booking.meeting_room.get_name(),
        ));
    }

    fn notify_cleanup_thread(&self, reason: CleanupContextStopReason) {
        *self.cleanup_context.wake_reason.lock().unwrap() = reason;
        self.cleanup_context.cv_wake_cleanup_thread.notify_one();
    }

    fn find_conflicting_rooms(
        data: &MeetingRoomSchedulerData,
        ts: &DateTimeSlot,
    ) -> HashSet<IntervalPayload> {
        let mut conflicting_room_names = HashSet::new();

        for overlapping_interval in data
            .i_tree
            .get_overlapping_intervals_with(ts.get_start_time(), ts.get_end_time())
        {
            conflicting_room_names.insert(overlapping_interval.payload);
        }

        return conflicting_room_names;
    }

    fn remove_expired_bookings_till(data: &mut MeetingRoomSchedulerData, date: IntervalType) {
        data.i_tree
            .get_intervals_ending_before(date)
            .iter()
            .for_each(|i| {
                data.i_tree.remove(i);
            });
    }

    fn run_cleanup(
        sync_data: Arc<RwLock<MeetingRoomSchedulerData>>,
        cleanup_context: Arc<CleanupContext>,
    ) {
        loop {
            let now = Local::now();
            let mut cleanup_required = false;

            {
                let mut write_data = sync_data.write().unwrap();

                // Remove all waiting times that are in the past
                while let Some(min_time) = write_data.end_times.peek() {
                    if min_time.0 < now {
                        write_data.end_times.pop();
                        cleanup_required = true;
                    } else {
                        break;
                    }
                }

                // Cleanup bookings that are in the past
                if cleanup_required {
                    MeetingRoomScheduler::remove_expired_bookings_till(
                        &mut write_data,
                        Local::now(),
                    );
                }
            }

            // Determine the time of the first ending booking
            let mut time_left_till_next_booking_ends = Duration::max_value();

            if let Some(min_time) = sync_data.read().unwrap().end_times.peek() {
                time_left_till_next_booking_ends = min_time.0 - now;
            }

            let mut wait_res = cleanup_context
                .cv_wake_cleanup_thread
                .wait_timeout_while(
                    cleanup_context.wake_reason.lock().unwrap(),
                    time_left_till_next_booking_ends.to_std().unwrap(),
                    |stop| *stop == CleanupContextStopReason::None,
                )
                .unwrap();

            if wait_res.1.timed_out() {
                let mut write_data = sync_data.write().unwrap();
                let first_end_time = write_data.end_times.peek().unwrap().0;

                MeetingRoomScheduler::remove_expired_bookings_till(&mut write_data, first_end_time);
                write_data.end_times.pop();
            } else {
                if *wait_res.0 == CleanupContextStopReason::Stop {
                    break;
                }

                // Signal was restart so we reset flag
                *wait_res.0 = CleanupContextStopReason::None;
            }
        }
    }
}
