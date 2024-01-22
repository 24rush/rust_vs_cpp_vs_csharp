use chrono::prelude::*;
use std::time::Duration;

#[derive(Clone, PartialEq)]
pub struct DateTimeSlot {
    time_point: DateTime<Local>,
    duration: Duration,
}

impl DateTimeSlot {
    pub fn from_ymd_hms(
        year: i32,
        month: u32,
        day: u32,
        hour: u32,
        min: u32,
        sec: u32,
        duration: Duration,
    ) -> Self {
        Self {
            time_point: Local
                .with_ymd_and_hms(year, month, day, hour, min, sec)
                .unwrap(),
            duration: duration,
        }
    }

    pub fn from_utc_time(tp: &DateTime<Local>, duration: Duration) -> Self {
        Self { time_point: *tp, duration: duration }
    }

    pub fn get_start_time(&self) -> DateTime<Local> {
        self.time_point
    }
    pub fn get_end_time(&self) -> DateTime<Local> {
        self.time_point + self.duration
    }

    pub fn get_length(&self) -> Duration {
        self.duration
    }
}

#[derive(Clone)]
pub struct MeetingRoom {
    name: String,
    seats: i8,
}

impl MeetingRoom {
    pub fn new(name: &str, seats: i8) -> Self {
        Self {
            name: name.to_string(),
            seats: seats,
        }
    }
    pub fn get_name(&self) -> String {
        self.name.clone()
    }
}

pub struct MeetingRoomBooking {
    pub meeting_room: MeetingRoom,
    pub time_slot: DateTimeSlot,
}

impl MeetingRoomBooking {
    pub fn new(mr: &MeetingRoom, ts: &DateTimeSlot) -> Self {
        Self {
            meeting_room: mr.clone(),
            time_slot: ts.clone(),
        }
    }
}
