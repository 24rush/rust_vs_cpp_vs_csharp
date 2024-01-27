use std::{
    ops::Add,
    thread::{self},
    time::Duration,
};

use chrono::Local;
use meeting_rooms::{
    types::{DateTimeSlot, MeetingRoom},
    MeetingRoomScheduler,
};

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
    let mut nr_threads = 1;
    let mut nr_intervals: u64 = 5000000;
    let mut nr_rooms = 3;

    let args: Vec<_> = std::env::args().collect();

    if args.len() > 1 && (args.len() - 1) % 2 == 0 {
        let next_token = |pos: usize| args.get(pos).unwrap().parse::<u64>().unwrap();

        for mut i in 0..args.len() {
            if args[i] == "-t" {
                i += 1;
                nr_threads = next_token(i);
            }

            if args[i] == "-i" {
                i += 1;
                nr_intervals = next_token(i);
            }

            if args[i] == "-r" {
                i += 1;
                nr_rooms = next_token(i);
            }
        }
    }

    println!(
        "Config: {0} threads | {1} rooms | {2} intervals",
        nr_threads, nr_rooms, nr_intervals
    );

    let scheduler = MeetingRoomScheduler::new();

    let mut meeting_rooms: Vec<MeetingRoom> = Vec::new();
    for i in 0..nr_rooms {
        meeting_rooms.push(MeetingRoom::new(
            &["#M".to_owned(), i.to_string().to_owned()].join(""),
            i as i8,
        ));
        scheduler.register_room(meeting_rooms.last().unwrap());
    }

    let mut indices: Vec<u64> = vec![];
    generate_indices(1, nr_intervals, &mut indices);

    thread::scope(|s| {
        for _ in 0..nr_threads {
            s.spawn(|| {
                let now = Local::now();

                for seconds in &indices {
                    let rnd_ts = DateTimeSlot::from_utc_time(
                        &now.add(Duration::from_secs(*seconds)),
                        Duration::from_millis(1),
                    );

                    let room = scheduler.request_room(&rnd_ts);

                    if let None = room {
                        println!("No rooms on {0}", rnd_ts.get_start_time());
                    }
                }
            });
        }
    });
}
