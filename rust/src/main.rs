use std::{ops::Add, thread, time::Duration};

use chrono::Local;
use meeting_rooms::{
    types::{DateTimeSlot, MeetingRoom},
    MeetingRoomScheduler,
};
use rand::Rng;

mod meeting_rooms;

fn generate_indices(low: u64, high: u64, indices: &mut Vec<u64>) {
    if low > high {
        return;
    }

    let mid = (low + high) / 2;
    indices.push(mid);

    generate_indices(low, mid - 1, indices);
    generate_indices(mid + 1, high, indices);
}

fn main() {
    let scheduler = MeetingRoomScheduler::new();

    let mut meeting_rooms: Vec<MeetingRoom> = Vec::new();
    for i in 0..3 {
        meeting_rooms.push(MeetingRoom::new(
            &["#M".to_owned(), i.to_string().to_owned()].join(""),
            i,
        ));
        scheduler.register_room(meeting_rooms.last().unwrap());
    }

    let request_room = move |nr_intervals: u64| {
        let now = Local::now();

        let mut indices: Vec<u64> = vec![];
        generate_indices(1, nr_intervals, &mut indices);

        for i in 0..nr_intervals {
            let rnd_ts = DateTimeSlot::from_utc_time(
                &now.add(Duration::from_secs(indices[i as usize])),
                Duration::from_millis(1),
            );

            let room = scheduler.request_room(&rnd_ts);

            if let None = room {
                println!("No rooms on {0}", rnd_ts.get_start_time());
            }            
        }
    };

    let t1 = thread::spawn(move || {
        request_room(2000000);
    });

    let _ = t1.join();
}
