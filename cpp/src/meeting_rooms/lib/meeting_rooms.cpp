#include <atomic>

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
    this->meetingRooms.insert(std::make_pair(m.getName(), m));
}

std::optional<MeetingRoomBooking> MeetingRoomScheduler::requestRoom(const DateTimeSlot &ts)
{
    static auto prevNow = system_clock::now();
    static std::atomic_int noBookings = 0;

    auto showNoBookingPerSec = []()
    {
        if ((system_clock::now() - prevNow) > seconds(1))
        {
            prevNow = system_clock::now();
            std::cout << noBookings << " bookings/sec \n";
            noBookings = 0;
        }
    };

    auto bookedRoomsInInterval = this->findConflictingRooms(ts);

    for (auto room : this->meetingRooms)
    {
        if (!bookedRoomsInInterval.contains(room.first))
        {
            noBookings++;
            showNoBookingPerSec();

            return this->bookRoom(room.second, ts);
        }
    }

    return std::nullopt;
}

std::optional<MeetingRoomBooking> MeetingRoomScheduler::requestRoom(const std::string &roomName, const DateTimeSlot &ts)
{
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
    this->iTree.insert({ts.getStartTime(), ts.getEndTime(), room.getName()});

    {
        std::lock_guard lock(this->lck_cleanup);

        this->endTimes.push(ts.getEndTime());

        if (ts.getEndTime() < this->endTimes.top())
        {
            this->restart = true;
            cv_wakeCleanupThread.notify_one();
        }
    }

    return MeetingRoomBooking{room, ts};
}

std::set<std::string> MeetingRoomScheduler::findConflictingRooms(const DateTimeSlot &ts)
{
    std::set<IntervalPayload> conflictingRoomNames;

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
        std::unique_lock lock(this->lck_cleanup);

        auto now = system_clock::now();

        if (this->endTimes.size() > 0 && this->endTimes.top() <= now)
        {
            auto lastPastInterval = this->endTimes.top();

            while (this->endTimes.size() > 0 && lastPastInterval <= now)
            {
                lastPastInterval = this->endTimes.top();
                this->endTimes.pop();
            }

            removeExpiredBookingsTill(lastPastInterval);
        }

        auto timeLeftTillNextBookingEnds = this->endTimes.size() == 0 ? nanoseconds(hours(1)) : this->endTimes.top() - system_clock::now();

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