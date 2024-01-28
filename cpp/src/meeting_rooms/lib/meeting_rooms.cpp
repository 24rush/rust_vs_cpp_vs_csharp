#include <iostream>
#include "../include/meeting_rooms.h"

MeetingRoomScheduler::MeetingRoomScheduler()
{
    this->cleanupThread = std::thread([this]
                                      { this->run_cleanup(); });
}

MeetingRoomScheduler::~MeetingRoomScheduler()
{
    this->stop = true;
    cv_wakeCleanupThread.notify_one();

    this->cleanupThread.join();
}

void MeetingRoomScheduler::registerRoom(const MeetingRoom &m)
{
    std::lock_guard guard_write(this->lck_meetingRooms);
    this->meetingRooms.insert(std::make_pair(m.getName(), m));
}

std::optional<MeetingRoomBooking> MeetingRoomScheduler::requestRoom(const DateTimeSlot &ts)
{     
    auto bookedRoomsInInterval = this->findConflictingRooms(ts);

    {
        std::shared_lock guard_read(this->lck_meetingRooms);
        for (auto room : this->meetingRooms)
        {
            if (!bookedRoomsInInterval.contains(room.first))
            {
                return this->bookRoom(room.second, ts);
            }
        }
    }

    return std::nullopt;
}

std::optional<MeetingRoomBooking> MeetingRoomScheduler::requestRoom(const std::string &roomName, const DateTimeSlot &ts)
{
    std::shared_lock guard_read(this->lck_meetingRooms);

    if (auto itMeetingRoom = this->meetingRooms.find(roomName); itMeetingRoom != this->meetingRooms.end())
    {
        if (!this->findConflictingRooms(ts).contains(itMeetingRoom->first))
            return this->bookRoom(itMeetingRoom->second, ts);
    }

    return std::nullopt;
}

std::string timePointToString(const std::chrono::sys_time<milliseconds> &time_point)
{
    auto s = duration_cast<seconds>(time_point.time_since_epoch()).count();
    return std::string(std::ctime(&s));
}

MeetingRoomBooking MeetingRoomScheduler::bookRoom(const MeetingRoom &room, const DateTimeSlot &ts)
{
    static auto prevNow = system_clock::now();
    static int noBookings = 0;
    static int totalBookings = 0;
    static bool firstBooking = true;

    auto showNoBookingPerSec = []()
    {
        if (firstBooking)
        {
            firstBooking = false;
            prevNow = system_clock::now();
        }
        else if ((system_clock::now() - prevNow) > seconds(1))
        {
            totalBookings += noBookings;
            std::cout << noBookings << " bookings/sec | total:  " << totalBookings << std::endl;

            prevNow = system_clock::now();
            noBookings = 0;
        }
    };

    this->iTree.insert({ts.getStartTime(), ts.getEndTime(), room.getName()});

    bool restart_cleanup = false;
    {
        std::lock_guard lock(this->lck_cleanup);

        // Check if current booking ends before previous minimum one
        restart_cleanup = this->endTimes.size() && ts.getEndTime() < this->endTimes.top();
        this->endTimes.push(ts.getEndTime());

        noBookings++;
        showNoBookingPerSec();
    }

    if (restart_cleanup)
    {
        this->restart = true;
        cv_wakeCleanupThread.notify_one();
    }

    return MeetingRoomBooking{room, ts};
}

std::set<std::string_view> MeetingRoomScheduler::findConflictingRooms(const DateTimeSlot &ts)
{
    std::set<std::string_view> conflictingRoomNames;

    for (auto overlappingInterval : this->iTree.getOverlappingIntervalsWith(ts.getStartTime(), ts.getEndTime()))
    {
        conflictingRoomNames.insert(overlappingInterval.payload);
    }

    return conflictingRoomNames;
}

void MeetingRoomScheduler::cancelBooking(const MeetingRoomBooking &booking)
{
    this->iTree.remove({booking.timeSlot.getStartTime(), booking.timeSlot.getEndTime(), booking.meetingRoom.getName()});
}

void MeetingRoomScheduler::run_cleanup()
{
    auto removeExpiredBookingsTill = [this](IntervalType tillEndTime)
    {
        for (auto booking : this->iTree.getIntervalsEndingBefore(tillEndTime))
        {
            this->iTree.remove(booking);
        }
    };

    do
    {
        auto now = system_clock::now();
        auto cleanup_required = false;

        std::unique_lock lock(this->lck_cleanup);

        while (this->endTimes.size() > 0 && this->endTimes.top() <= now)
        {
            this->endTimes.pop();
            cleanup_required = true;
        }

        if (cleanup_required)
        {
            removeExpiredBookingsTill(time_point_cast<std::chrono::milliseconds>(now));
        }

        auto timeLeftTillNextBookingEnds = this->endTimes.size() == 0 ? nanoseconds(hours(1)) : this->endTimes.top() - now;

        auto waitRes = cv_wakeCleanupThread.wait_for(lock, timeLeftTillNextBookingEnds, [&]
                                                     { return this->stop || this->restart; });

        if (!waitRes) // timeout expired
        {
            removeExpiredBookingsTill(this->endTimes.top());
            this->endTimes.pop();
        }

        this->restart = false;

        if (this->stop)
            break;

    } while (true);
}