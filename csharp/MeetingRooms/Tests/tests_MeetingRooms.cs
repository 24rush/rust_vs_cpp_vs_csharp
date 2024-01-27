namespace Tests;

[TestClass]
public class Tests_MeetingRooms
{
    [TestMethod]
    public void datetimeslot()
    {
        var ts = new DateTimeSlot(2023, 11, 28, 3, 15, 60);
    }

    [TestMethod]
    public void book()
    {
        var m1 = new MeetingRoom()
        {
            Name = "M1",
            Seats = 4
        };

        var startDate = DateTime.Now.AddSeconds(10);
        var slot1 = new DateTimeSlot(startDate, 60);

        var scheduler = new MeetingRooms();

        var booking = scheduler.RequestRoom(slot1);
        Assert.IsTrue(booking == null);

        scheduler.RegisterRoom(m1);
        booking = scheduler.RequestRoom(slot1);
        Assert.IsFalse(booking == null);

        var booking1 = scheduler.RequestRoom(slot1);
        Assert.IsTrue(booking1 == null);

        scheduler.CancelBooking(booking);
        booking = scheduler.RequestRoom(slot1);
        Assert.IsFalse(booking == null);

        var slot2 = new DateTimeSlot(startDate, 15);
        var booking2 = scheduler.RequestRoom(slot2);
        Assert.IsTrue(booking2 == null);
    }

    [TestMethod]
    public void book2()
    {
        var m1 = new MeetingRoom()
        {
            Name = "M1",
            Seats = 4
        };
        var m2 = new MeetingRoom()
        {
            Name = "M2",
            Seats = 8
        };

        var startDate = DateTime.Now.AddSeconds(10);

        var slot1 = new DateTimeSlot(startDate, 60);
        var scheduler = new MeetingRooms();
        scheduler.RegisterRoom(m1);
        scheduler.RegisterRoom(m2);

        var booking = scheduler.RequestRoom(slot1);
        Assert.IsTrue(booking != null);

        booking = scheduler.RequestRoom(slot1);
        Assert.IsTrue(booking != null);

        var slot2 = new DateTimeSlot(startDate, 15);
        var booking2 = scheduler.RequestRoom(slot2);
        Assert.IsTrue(booking2 == null);
    }


    [TestMethod]
    public void book_room_w_name()
    {
        var m1 = new MeetingRoom()
        {
            Name = "M1",
            Seats = 4
        };

        var startDate = DateTime.Now.AddSeconds(10);

        var slot1 = new DateTimeSlot(startDate, 60);
        var scheduler = new MeetingRooms();
        scheduler.RegisterRoom(m1);

        var booking = scheduler.RequestRoom(slot1);
        Assert.IsTrue(booking != null);

        booking = scheduler.RequestRoom("M1", slot1);
        Assert.IsTrue(booking == null);

        var slot2 = new DateTimeSlot(startDate, 15);
        var booking2 = scheduler.RequestRoom(slot2);
        Assert.IsTrue(booking2 == null);
    }

    [TestMethod]
    public void book_room_wait_cleanup()
    {
        var scheduler = new MeetingRooms();

        for (var i = 1; i < 3; i++)
        {
            var m = new MeetingRoom()
            {
                Name = "M" + i.ToString(),
                Seats = 4
            };
            scheduler.RegisterRoom(m);
        }

        var now = DateTime.Now;

        var requestRoom = (int nr_intervals) =>
        {
            for (var i = 1; i < nr_intervals; i++)
            {
                var slot1 = new DateTimeSlot(now.AddSeconds(i), 1);

                var booking = scheduler.RequestRoom(slot1);                
            }
        };

        var t1 = new Thread(() => requestRoom(2000));

        t1.Start();
        t1.Join();
    }
}