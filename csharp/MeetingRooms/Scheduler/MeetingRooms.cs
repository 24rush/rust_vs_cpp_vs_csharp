
using System.Collections.Concurrent;
using Scheduler;

public class DateTimeSlot
{
    public DateTimeSlot(int year, int month, int day, int hour, int minute, double duration)
    {
        this.timePoint = new DateTime(year, month, day, hour, minute, 0);
        this.duration = duration;
    }

    public DateTimeSlot(DateTime dt, double duration)
    {
        this.timePoint = dt;
        this.duration = duration;
    }

    public DateTime GetStartTime() { return this.timePoint; }
    public DateTime GetEndTime() { return this.timePoint.AddMilliseconds(this.duration); }

    private DateTime timePoint;
    private double duration;
}

public class MeetingRoom
{
    public string Name { get; set; } = "";
    public int Seats { get; set; }
}

public class MeetingRoomBooking
{
    public MeetingRoomBooking(MeetingRoom mr, DateTimeSlot ts)
    {
        MeetingRoom = mr;
        TimeSlot = ts;
    }

    public MeetingRoom MeetingRoom { get; }
    public DateTimeSlot TimeSlot { get; }
}

public class MeetingRooms
{
    private ConcurrentDictionary<string, MeetingRoom> meetingRooms = new ConcurrentDictionary<string, MeetingRoom>();

    private IntervalTree<DateTime, string> intervalTree = new IntervalTree<DateTime, string>();

    private PriorityQueue<DateTime, DateTime> endTimes = new PriorityQueue<DateTime, DateTime>();

    private Thread cleanupThread;
    readonly object wakeCleanupThread = new object();
    private bool stop = false;

    public MeetingRooms()
    {
        cleanupThread = new Thread(runCleanup);
        cleanupThread.Start();
    }

    ~MeetingRooms()
    {
        Stop();
    }

    public void Stop()
    {
        stop = true;
        lock (wakeCleanupThread)
        {
            Monitor.Pulse(wakeCleanupThread);
        }

        cleanupThread.Join();
    }

    public void RegisterRoom(MeetingRoom mr)
    {
        meetingRooms.TryAdd(mr.Name, mr);
    }

    public void CancelBooking(MeetingRoomBooking booking)
    {
        intervalTree.Remove(new Data<DateTime, string>(booking.TimeSlot.GetStartTime(), booking.TimeSlot.GetEndTime(), booking.MeetingRoom.Name));
    }

    public MeetingRoomBooking? RequestRoom(DateTimeSlot ts)
    {
        var bookedRoomsInInterval = findConflictingRooms(ts);        

        foreach (var room in meetingRooms)
        {
            if (!bookedRoomsInInterval.Contains(room.Key))
            {
                return bookRoom(room.Value, ts);                
            }
        }

        return null;
    }

    public MeetingRoomBooking? RequestRoom(string roomName, DateTimeSlot ts)
    {
        MeetingRoom? mr;        

        if (meetingRooms.TryGetValue(roomName, out mr))
        {
            if (!findConflictingRooms(ts).Contains(roomName))
            {
                return bookRoom(mr, ts);
            }
        }

        return null;
    }

    static int noBookings = 0;
    static int totalBookings = 0;
    static bool firstBooking = true;

    static DateTime prevNow = DateTime.Now;


    private MeetingRoomBooking bookRoom(MeetingRoom mr, DateTimeSlot ts)
    {
        var showNoBookingsPerSec = () =>
        {
            if (firstBooking)
            {
                firstBooking = false;
                prevNow = DateTime.Now;
                return;
            }

            noBookings++;

            if (DateTime.Now - prevNow > TimeSpan.FromSeconds(1))
            {
                totalBookings += noBookings;
                Console.WriteLine("{0} bookings/sec | total: {1}", noBookings, totalBookings);

                prevNow = DateTime.Now;
                noBookings = 0;
            }
        };

        intervalTree.Insert(new Data<DateTime, string>(ts.GetStartTime(), ts.GetEndTime(), mr.Name));

        DateTime prevFirstItem;
        bool needsRestart;

        lock (wakeCleanupThread)
        {
            if (endTimes.TryPeek(out prevFirstItem, out prevFirstItem))
            {
                // If new item ends sooner than the current min one then restart
                needsRestart = prevFirstItem > ts.GetEndTime();
            }
            else
            {
                // If empty queue then force restart as we need to wait on the newly added item
                needsRestart = true;
            }

            endTimes.Enqueue(ts.GetEndTime(), ts.GetEndTime());

            //showNoBookingsPerSec();

            if (needsRestart)
            {
                Monitor.Pulse(wakeCleanupThread);
            }
        }

        return new MeetingRoomBooking(mr, ts);
    }

    HashSet<string> findConflictingRooms(DateTimeSlot ts)
    {
        var conflictingRoomsName = new HashSet<string>();

        intervalTree.SearchOverlappingIntervals(ts.GetStartTime(), ts.GetEndTime()).ForEach(overlappingInterval =>
        {
            conflictingRoomsName.Add(overlappingInterval.Payload);
        });

        return conflictingRoomsName;
    }

    void runCleanup()
    {
        var removeExpiredBookingsTill = (DateTime tillEndTime) =>
        {
            intervalTree.SearchIntervalsEndingBefore(tillEndTime).ForEach(booking =>
            {
                intervalTree.Remove(booking);
            });
        };

        do
        {
            DateTime minEndTime;

            var now = DateTime.Now;
            var cleanupRequired = false;

            lock (wakeCleanupThread)
            {
                // Determine if there are intervals ending in the past (before Now)
                while (endTimes.TryPeek(out minEndTime, out minEndTime) && minEndTime <= now)
                {
                    endTimes.Dequeue();
                    cleanupRequired = true;
                }

                if (cleanupRequired)
                {
                    // Remove intervals ending Now
                    removeExpiredBookingsTill(now);
                }

                TimeSpan timeLeftTillNextBookindEnds = endTimes.Count == 0 ? TimeSpan.FromMilliseconds(-1) : endTimes.Peek() - now;

                var waitRes = Monitor.Wait(wakeCleanupThread, timeLeftTillNextBookindEnds);

                if (!waitRes) // timeout expired
                {
                    removeExpiredBookingsTill(endTimes.Peek());
                    endTimes.Dequeue();
                }

                if (stop)
                    break;
            }

        } while (true);
    }
}