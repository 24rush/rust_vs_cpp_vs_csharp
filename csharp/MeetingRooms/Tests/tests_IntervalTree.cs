namespace Tests;
using Scheduler;
using IntInterval = Scheduler.Data<int, int>;

[TestClass]
public class Tests_IntervalTree
{
    [TestMethod]
    public void SearchOverlappingIntervals()
    {
        var intervalTree = new IntervalTree<int, int>();

        var intervals = new (int, int, int)[] { (1, 20, 4), (10, 30, 5), (17, 19, 6), (5, 20, 7), (12, 15, 8), (30, 40, 9) };
        foreach (var interval in intervals)
        {
            intervalTree.Insert(new IntInterval(interval.Item1, interval.Item2, interval.Item3));
        }

        var overlaps = intervalTree.SearchOverlappingIntervals(6, 7);

        Assert.IsTrue(overlaps.Count() == 2);
    }

    [TestMethod]
    public void SearchIntervalsEndingBefore()
    {
        var intervalTree = new IntervalTree<int, int>();

        var intervals = new (int, int, int)[] { (0, 1, 1), (0, 1, 2), (3, 7, 3), (2, 6, 4), (10, 15, 5), (5, 6, 6), (4, 100, 7) };
        foreach (var interval in intervals)
        {
            intervalTree.Insert(new IntInterval(interval.Item1, interval.Item2, interval.Item3));
        }

        var overlaps = intervalTree.SearchOverlappingIntervals(6, 7);

        Assert.IsTrue(intervalTree.SearchIntervalsEndingBefore(1).Count() == 2);
        Assert.IsTrue(intervalTree.SearchIntervalsEndingBefore(2).Count() == 2);
        Assert.IsTrue(intervalTree.SearchIntervalsEndingBefore(6).Count() == 4);
        Assert.IsTrue(intervalTree.SearchIntervalsEndingBefore(15).Count() == 6);
    }

    [TestMethod]
    public void SearchIntervalsEndingBefore_And_Remove()
    {
        var intervalTree = new IntervalTree<int, int>();

        var intervals = new (int, int, int)[] { (0, 1, 1), (0, 1, 2), (3, 7, 3), (2, 6, 4), (10, 15, 5), (5, 6, 6), (4, 100, 7) };
        foreach (var interval in intervals)
        {
            intervalTree.Insert(new IntInterval(interval.Item1, interval.Item2, interval.Item3));
        }

        Assert.IsTrue(intervalTree.SearchOverlappingIntervals(0, 1).Count() == 2);
        Assert.IsTrue(intervalTree.SearchOverlappingIntervals(2, 7).Count() == 4);
        Assert.IsTrue(intervalTree.SearchOverlappingIntervals(1, 2).Count() == 0);
        Assert.IsTrue(intervalTree.SearchOverlappingIntervals(1, 100).Count() == 5);
        Assert.IsTrue(intervalTree.SearchOverlappingIntervals(5, 12).Count() == 5);
        Assert.IsTrue(intervalTree.SearchOverlappingIntervals(101, 102).Count() == 0);
        Assert.IsTrue(intervalTree.SearchOverlappingIntervals(6, 2).Count() == 4);

        intervalTree.Remove(new IntInterval(0, 1, 1));
        Assert.IsTrue(intervalTree.SearchOverlappingIntervals(-1, 1).Count() == 1);

        intervalTree.Remove(new IntInterval(2, 6, 4));
        Assert.IsTrue(intervalTree.SearchOverlappingIntervals(2, 3).Count() == 0);

        intervalTree.Remove(new IntInterval(0, 1, 2));
        Assert.IsTrue(intervalTree.SearchOverlappingIntervals(-1, 1).Count() == 0);
    }
}