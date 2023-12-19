namespace Scheduler;

using IntervalType = Interval<int, int>;
using IntervalTreeNodeType = IntervalTreeNode<int, int>;

// Define an Interval class
public class Interval<TType, TPayload>
{
    public TType Start { get; set; }
    public TType End { get; set; }

    public TPayload Payload { get; set; }

    public Interval(TType start, TType end, TPayload payload)
    {
        Start = start;
        End = end;
        Payload = payload;
    }
}

// Define a Node class for the Interval Tree
public class IntervalTreeNode<TType, TPayload>
{
    public Interval<TType, TPayload> Interval { get; set; }
    public IntervalTreeNode<TType, TPayload>? Left { get; set; }
    public IntervalTreeNode<TType, TPayload>? Right { get; set; }
    public TType MaxEnd { get; set; }

    public IntervalTreeNode(Interval<TType, TPayload> interval)
    {
        Interval = interval;
        MaxEnd = interval.End;
    }
}

// Define the IntervalTree class
public class IntervalTree
{

    private IntervalTreeNodeType? root;

    public IntervalTree()
    {
        root = null;
    }

    // Insert a new interval into the tree
    public void Insert(IntervalType interval)
    {
        root = InsertInterval(root, interval);
    }

    private IntervalTreeNodeType InsertInterval(IntervalTreeNodeType? node, IntervalType interval)
    {
        if (node == null)
        {
            return new IntervalTreeNodeType(interval);
        }

        // Update MaxEnd for this node
        node.MaxEnd = Math.Max(node.MaxEnd, interval.End);

        // Decide whether to go left or right
        if (interval.Start < node.Interval.Start)
        {
            node.Left = InsertInterval(node.Left, interval);
        }
        else
        {
            node.Right = InsertInterval(node.Right, interval);
        }

        return node;
    }

    // Search for intervals that overlap with a given interval
    public List<IntervalType> SearchOverlappingIntervals(IntervalType interval)
    {
        List<IntervalType> result = new List<IntervalType>();
        SearchOverlappingIntervals(root, interval, result);
        return result;
    }

    private void SearchOverlappingIntervals(IntervalTreeNodeType? node, IntervalType interval, List<IntervalType> result)
    {
        if (node == null)
        {
            return;
        }

        // Check if the interval overlaps with the current node's interval
        if (interval.Start <= node.Interval.End && interval.End >= node.Interval.Start)
        {
            result.Add(node.Interval);
        }

        // Check if the left subtree may contain overlapping intervals
        if (node.Left != null && node.Left.MaxEnd >= interval.Start)
        {
            SearchOverlappingIntervals(node.Left, interval, result);
        }

        // Check if the right subtree may contain overlapping intervals
        if (node.Right != null && interval.End >= node.Right.Interval.Start)
        {
            SearchOverlappingIntervals(node.Right, interval, result);
        }
    }
}


