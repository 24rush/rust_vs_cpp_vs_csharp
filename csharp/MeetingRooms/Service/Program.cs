
class Program
{
    static void generate_indices(int low, int high, List<int> indices)
    {
        if (low > high)
            return;

        int mid = (low + high) / 2;
        indices.Add(mid);

        generate_indices(low, mid - 1, indices);
        generate_indices(mid + 1, high, indices);
    }

    static void Main(string[] args)
    {
        var nr_threads = 1;
        var nr_intervals = 50000;
        var nr_rooms = 3;

        if (args.Length > 1 && (args.Length % 2 == 0))
        {
            var next_token = (int pos) =>
            {
                return int.Parse(args[pos + 1]);
            };

            for (var i = 0; i < args.Length; i++)
            {
                if (args[i] == "-t")
                {
                    nr_threads = next_token(i++);
                }

                if (args[i] == "-i")
                {
                    nr_intervals = next_token(i++);
                }

                if (args[i] == "-r")
                {
                    nr_rooms = next_token(i++);
                }
            }
        }

        Console.WriteLine("Config: {0} threads | {1} rooms | {2} intervals", nr_threads, nr_rooms, nr_intervals);

        var scheduler = new MeetingRooms();

        for (var i = 0; i < nr_rooms; i++)
        {
            var m = new MeetingRoom()
            {
                Name = "M" + i.ToString(),
                Seats = i
            };
            scheduler.RegisterRoom(m);
        }

        var indices = new List<int>();
        generate_indices(1, nr_intervals, indices);

        var requestRoom = () =>
        {
            var now = DateTime.Now;

            foreach (var seconds in indices)
            {
                var slot1 = new DateTimeSlot(now.AddSeconds(seconds), 1);

                var booking = scheduler.RequestRoom(slot1);
            }
        };

        var threads = new List<Thread>();

        for (var tn = 0; tn < nr_threads; tn++)
        {
            var t = new Thread(() => requestRoom());
            threads.Add(t);
            t.Start();
        }

        foreach (var t in threads)
            t.Join();
            
        scheduler.Stop();
    }
}