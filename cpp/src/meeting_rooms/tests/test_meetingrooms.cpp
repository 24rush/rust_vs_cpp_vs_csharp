
#include <gtest/gtest.h>

#include <iostream>
#include <vector>
#include <random>

#include "../include/meeting_rooms.h"

int64_t generate_rnd_duration(int maxSpan)
{
    std::random_device rd;
    std::mt19937 gen(rd());

    std::uniform_int_distribution<int64_t> d(0, maxSpan);

    return d(gen);
}

TEST(meeting_rooms, timeSlot_ctor)
{
    DateTimeSlot ts(2023y / 11 / 28, 3u, 15u, 60u);

    ASSERT_EQ(ts.getLength().count(), 60);

    DateTimeSlot ts2(2023y / 11 / 28, 3u, 15u, 60u);
    EXPECT_TRUE(ts == ts2);
}

TEST(meeting_rooms, MeetingRoom_book)
{
    MeetingRoom m1("M1", 4);

    auto slot1 = DateTimeSlot(2023y / 11 / 28, 15u, 30u, 60u);

    MeetingRoomScheduler scheduler;

    auto booking = scheduler.requestRoom(slot1);
    EXPECT_FALSE(booking.has_value());

    scheduler.registerRoom(m1);
    booking = scheduler.requestRoom(slot1);
    EXPECT_TRUE(booking.has_value());

    auto booking1 = scheduler.requestRoom(slot1);
    EXPECT_FALSE(booking1.has_value());

    scheduler.cancelBooking(booking.value());
    booking = scheduler.requestRoom(slot1);
    EXPECT_TRUE(booking.has_value());

    auto slot2 = DateTimeSlot(2023y / 11 / 28, 15u, 30u, 15u);
    auto booking2 = scheduler.requestRoom(slot2);
    EXPECT_FALSE(booking2.has_value());
}

TEST(meeting_rooms, MeetingRoom_book_2)
{
    MeetingRoom m1("M1", 4);
    MeetingRoom m2("M2", 8);

    auto slot1 = DateTimeSlot(2023y / 11 / 28, 15u, 30u, 60u);

    MeetingRoomScheduler scheduler;
    scheduler.registerRoom(m1);
    scheduler.registerRoom(m2);

    auto room = scheduler.requestRoom(slot1);
    EXPECT_TRUE(room.has_value());

    room = scheduler.requestRoom(slot1);
    EXPECT_TRUE(room.has_value());

    auto slot2 = DateTimeSlot(2023y / 11 / 28, 15u, 30u, 15u);
    room = scheduler.requestRoom(slot2);
    EXPECT_FALSE(room.has_value());
}

TEST(meeting_rooms, MeetingRoom_book_with_room_name)
{
    MeetingRoom m1("M1", 4);

    auto slot1 = DateTimeSlot(2023y / 11 / 28, 15u, 30u, 60u);

    MeetingRoomScheduler scheduler;
    scheduler.registerRoom(m1);

    auto room = scheduler.requestRoom(slot1);
    EXPECT_TRUE(room.has_value());

    room = scheduler.requestRoom("M1", slot1);
    EXPECT_FALSE(room.has_value());

    auto slot2 = DateTimeSlot(2023y / 11 / 28, 15u, 30u, 15u);
    room = scheduler.requestRoom(slot2);
    EXPECT_FALSE(room.has_value());
}

TEST(meeting_rooms, MeetingRoom_book_3)
{
    MeetingRoomScheduler scheduler;

    std::list<MeetingRoom> meetingRooms;
    for (size_t i = 0; i < 3; i++)
    {
        meetingRooms.push_back({"#M" + std::to_string(i), i});
        scheduler.registerRoom(meetingRooms.back());
    }

    for (size_t i = 0; i < 100; i++)
    {
        auto rnd_ts = DateTimeSlot(system_clock::now() + minutes(60 * i), 60);
        auto booking = scheduler.requestRoom(rnd_ts);

        EXPECT_TRUE(booking.has_value());
    }
}

TEST(meeting_rooms, remove_passed_bookings)
{
    MeetingRoomScheduler scheduler;

    std::list<MeetingRoom> meetingRooms;
    for (size_t i = 0; i < 3; i++)
    {
        meetingRooms.push_back({"#M" + std::to_string(i), i});
        scheduler.registerRoom(meetingRooms.back());
    }

    auto requestRoom = [&scheduler]()
    {
        for (size_t i = 0; i < 10; i++)
        {
            auto now = system_clock::now();
            auto millis = milliseconds(i);
            auto rnd_ts = DateTimeSlot((now + millis), 1);

            auto room = scheduler.requestRoom(rnd_ts);

            if (room.has_value())
            {
                // std::cout << "Room " << room->meetingRoom.getName() << "\n" << room->timeSlot.toString();
            }
            else
            {
                // std::cout << "No rooms on " << rnd_ts.toString();
            }
        }
    };

    std::thread t1(requestRoom);
    std::thread t2(requestRoom);
    std::thread t3(requestRoom);

    t1.join();
    t2.join();
    t3.join();
}