#include <string_view>
#include <optional>
#include <unordered_map>
#include <functional>
#include <queue>

#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>

#include <chrono>
using namespace std::chrono;

#include "interval_tree.hpp"
#include <iostream>

class DateTimeSlot
{
public:
    DateTimeSlot() = default;

    DateTimeSlot(const system_clock::time_point &timePoint, unsigned int min_duration) : timePoint(time_point_cast<std::chrono::milliseconds>(timePoint)), duration(min_duration)
    {
    }

    DateTimeSlot(year_month_day ymd, unsigned int hour, unsigned int minute, unsigned int min_duration) : timePoint(sys_days(ymd) + hours(hour) + minutes(minute)),
                                                                                                          duration(minutes(min_duration))
    {
    }

    auto getLength() { return this->duration; }

    const auto getStartTime() const { return this->timePoint; }
    const auto getEndTime() const { return this->timePoint + this->duration; }

    bool operator==(const DateTimeSlot &other)
    {
        return this->timePoint == other.timePoint && this->duration == other.duration;
    }

    std::string toString()
    {
        auto start = duration_cast<seconds>(this->getStartTime().time_since_epoch()).count();
        auto end = duration_cast<seconds>(this->getEndTime().time_since_epoch()).count();

        return std::string(std::ctime(&start)) + std::string(std::ctime(&end));
    }

protected:
    sys_time<milliseconds> timePoint;
    milliseconds duration;
};

class MeetingRoom
{
public:
    MeetingRoom() = default;
    MeetingRoom(MeetingRoom &&) = default;
    MeetingRoom(const MeetingRoom &) = default;
    MeetingRoom &operator=(MeetingRoom &) = default;
    MeetingRoom &operator=(MeetingRoom &&) = default;

    MeetingRoom(const std::string &name, size_t seats) : name(name), seats(seats)
    {
    }

    auto getName() const { return this->name; }

protected:
    std::string name;
    size_t seats;
};

struct MeetingRoomBooking
{
    MeetingRoom meetingRoom;
    DateTimeSlot timeSlot;
};

class MeetingRoomScheduler
{
public:
    MeetingRoomScheduler();

    void registerRoom(const MeetingRoom &m);

    std::optional<MeetingRoomBooking> requestRoom(const DateTimeSlot &ts);
    std::optional<MeetingRoomBooking> requestRoom(const std::string& roomName, const DateTimeSlot &ts);

    void cancelBooking(const MeetingRoomBooking &booking);

    virtual ~MeetingRoomScheduler();

protected:
    using IntervalType = sys_time<milliseconds>; // meeting timestamp
    using IntervalPayload = std::string; // meeting room name

    // Storage for booked intervals
    IntervalTree<IntervalType, IntervalPayload> iTree;

    // Heap for cleaning up past meetings
    std::priority_queue<IntervalType, std::vector<IntervalType>, std::greater<IntervalType>> endTimes;

    // Storage of registered meeting rooms
    std::unordered_map<IntervalPayload, MeetingRoom> meetingRooms;

    mutable std::recursive_mutex lck_cleanup;
    mutable std::condition_variable_any cv_wakeCleanupThread;
    std::atomic_bool stop = false;
    std::atomic_bool restart = false;

    std::thread cleanupThread;
    void run_cleanup();

    std::set<IntervalPayload> findConflictingRooms(const DateTimeSlot &ts);
    MeetingRoomBooking bookRoom(const MeetingRoom &room, const DateTimeSlot &ts);
};