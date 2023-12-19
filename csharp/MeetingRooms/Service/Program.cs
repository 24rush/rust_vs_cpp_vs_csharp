using Scheduler;
using IntInterval = Scheduler.Interval<int, int>;

class Program
{
    static void Main()
    {
        // Create an interval tree
        IntervalTree intervalTree = new IntervalTree();

        // Insert intervals into the tree
        intervalTree.Insert(new IntInterval(15, 20, 1));
        intervalTree.Insert(new IntInterval(10, 30, 1));
        intervalTree.Insert(new IntInterval(17, 19, 3));
        intervalTree.Insert(new IntInterval(5, 20, 4));
        intervalTree.Insert(new IntInterval(12, 15, 5));
        intervalTree.Insert(new IntInterval(30, 40, 6));

        // Search for overlapping intervals with a given interval
        IntInterval searchInterval = new IntInterval(14, 16, 7);
        List<IntInterval> overlappingIntervals = intervalTree.SearchOverlappingIntervals(searchInterval);

        // Print the result
        Console.WriteLine("Overlapping Intervals with [{0}, {1}]:", searchInterval.Start, searchInterval.End);
        foreach (var overlappingInterval in overlappingIntervals)
        {
            Console.WriteLine("[{0}, {1}]", overlappingInterval.Start, overlappingInterval.End);
        }
    }
}