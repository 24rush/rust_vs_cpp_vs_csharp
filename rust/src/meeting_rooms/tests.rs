use std::{time::Duration, ops::Add};
use chrono::Local;
use rand::Rng;

use super::types::{DateTimeSlot, MeetingRoom};
use crate::meeting_rooms::MeetingRoomScheduler;

#[cfg(test)]

#[test]
fn datetimeslot() {
    use chrono::Local;

    let ts0 = DateTimeSlot::from_ymd_hms(2023, 11, 28, 3, 15, 0, Duration::new(5, 0));
    assert_eq!(ts0.get_length(), Duration::from_secs(5));

    let ts1 = DateTimeSlot::from_ymd_hms(2023, 11, 28, 3, 15, 0, Duration::new(5, 0));
    assert!(ts0 == ts1);

    let ts2 = DateTimeSlot::from_utc_time(&Local::now(), Duration::from_secs(60 * 5));
    assert_eq!(ts2.get_length(), Duration::from_secs(60 * 5));
}

#[test]
fn scheduler() {
    let m1: MeetingRoom = MeetingRoom::new("M1", 4);
    let slot1 = DateTimeSlot::from_ymd_hms(2023, 11, 28, 15, 30, 0, Duration::new(60, 0));

    let scheduler = MeetingRoomScheduler::new();

    let mut booking = scheduler.request_room(&slot1);
    assert!(booking.is_none());

    scheduler.register_room(&m1);
    booking = scheduler.request_room(&slot1);
    assert!(booking.is_some());

    let booking1 = scheduler.request_room(&slot1);
    assert!(booking1.is_none());

    scheduler.cancel_booking(&booking.unwrap());
    booking = scheduler.request_room(&slot1);
    assert!(booking.is_some());

    let slot2 = DateTimeSlot::from_ymd_hms(2023, 11, 28, 15, 30, 0, Duration::new(15, 0));
    let booking2 = scheduler.request_room(&slot2);
    assert!(booking2.is_none());

    scheduler.cancel_booking(&booking.unwrap());
    let booking3 = scheduler.request_room(&slot2);
    assert!(booking3.is_some());

    drop(scheduler);
}

#[test]
fn remove_passed_bookings() {
    let scheduler = MeetingRoomScheduler::new();

    let mut meeting_rooms: Vec<MeetingRoom> = Vec::new();
    for i in 1..4 {
        meeting_rooms.push(MeetingRoom::new(
            &["#M".to_owned(), i.to_string().to_owned()].join(""),
            i,
        ));
        scheduler.register_room(meeting_rooms.last().unwrap());
        //println!("Registered {0}", meeting_rooms.last().unwrap().get_name());
    }

    let request_room = |i: u64| {
        std::thread::sleep(Duration::from_millis(i * 100));

        for _i in 0..100 
        {
            let now = Local::now();

            let rnd_ms = rand::thread_rng().gen_range(0..6);

            let rnd_ts = DateTimeSlot::from_utc_time(
                &now.add(Duration::from_secs(rnd_ms)),
                Duration::from_millis(10),
            );

            //println!("Request {0}", rnd_ts.get_start_time());

            let room = scheduler.request_room(&rnd_ts);
            
            if let Some(avail_room) = room {
               /* println!(
                    "Room {0} {1}",
                    avail_room.meeting_room.get_name(),
                    avail_room.time_slot.get_start_time()
                );*/
            } else {
                //println!("No rooms on {0}", rnd_ts.get_start_time());
            }
        }
    };

    request_room(1);

    std::thread::sleep(Duration::from_secs(2));    

    drop(scheduler);
}
