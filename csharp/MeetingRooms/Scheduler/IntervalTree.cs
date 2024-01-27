namespace Scheduler;

// Define an Interval class
public class Data<IntervalType, PayloadType>
{
    public IntervalType Start { get; set; }
    public IntervalType End { get; set; }
    public PayloadType Payload { get; set; }

    public Data(IntervalType start, IntervalType end, PayloadType payload)
    {
        Start = start;
        End = end;
        Payload = payload;
    }
}

// Define a Node class for the Interval Tree
public class IntervalTreeNode<IntervalType, PayloadType>
{
    public IntervalType Start { get; set; }
    public IntervalType End { get; set; }

    public HashSet<PayloadType> Payloads { get; set; }
    public IntervalTreeNode<IntervalType, PayloadType>? Left { get; set; }
    public IntervalTreeNode<IntervalType, PayloadType>? Right { get; set; }
    public IntervalType MaxEnd { get; set; }

    public IntervalTreeNode(Data<IntervalType, PayloadType> interval)
    {
        Start = interval.Start;
        End = interval.End;
        Payloads = new HashSet<PayloadType> { interval.Payload };
        MaxEnd = interval.End;
    }
}

// Define the IntervalTree class
public class IntervalTree<IntervalType, PayloadType> where IntervalType : IComparable
{
    private IntervalTreeNode<IntervalType, PayloadType>? root;
    private ReaderWriterLockSlim rootSync = new ReaderWriterLockSlim();

    public IntervalTree()
    {
        root = null;
    }

    // Insert a new interval into the tree
    public void Insert(Data<IntervalType, PayloadType> interval)
    {
        rootSync.EnterWriteLock();
        root = InsertInterval(root, interval);
        rootSync.ExitWriteLock();
    }

    public void Remove(Data<IntervalType, PayloadType> interval)
    {
        rootSync.EnterWriteLock();
        root = RemoveNode(root, interval.Start, interval.End, interval.Payload);
        rootSync.ExitWriteLock();
    }

    private IntervalTreeNode<IntervalType, PayloadType> InsertInterval(IntervalTreeNode<IntervalType, PayloadType>? node, Data<IntervalType, PayloadType> interval)
    {
        if (node == null)
        {
            return new IntervalTreeNode<IntervalType, PayloadType>(interval);
        }

        if (interval.Start.CompareTo(node.Start) == 0 && interval.End.CompareTo(node.End) == 0)
        {
            node.Payloads.Add(interval.Payload);
            return node;
        }

        // Decide whether to go left or right
        if (interval.Start.CompareTo(node.Start) < 0)
        {
            node.Left = InsertInterval(node.Left, interval);
        }
        else
        {
            node.Right = InsertInterval(node.Right, interval);
        }

        // Update MaxEnd for this node        
        node.MaxEnd = node.MaxEnd.CompareTo(interval.End) > 0 ? node.MaxEnd : interval.End;

        return node;
    }

    // Search for intervals that overlap with a given interval
    public List<Data<IntervalType, PayloadType>> SearchOverlappingIntervals(IntervalType low, IntervalType high)
    {
        var result = new List<Data<IntervalType, PayloadType>>();
        var isLowHigh = low.CompareTo(high) < 0;

        rootSync.EnterReadLock();
        SearchOverlappingIntervals(root, isLowHigh ? low : high, isLowHigh ? high : low, result);
        rootSync.ExitReadLock();

        return result;
    }

    public List<Data<IntervalType, PayloadType>> SearchIntervalsEndingBefore(IntervalType high)
    {
        var result = new List<Data<IntervalType, PayloadType>>();

        rootSync.EnterReadLock();
        SearchIntervalsEndingBefore(root, high, result);
        rootSync.ExitReadLock();

        return result;
    }

    private void SearchOverlappingIntervals(IntervalTreeNode<IntervalType, PayloadType>? node, IntervalType low, IntervalType high, List<Data<IntervalType, PayloadType>> result)
    {
        if (node == null)
            return;

        // Check if the interval overlaps with the current node's interval
        if (low.CompareTo(node.End) < 0 && high.CompareTo(node.Start) > 0)
        {
            foreach (var payload in node.Payloads)
                result.Add(new Data<IntervalType, PayloadType>(node.Start, node.End, payload));
        }

        // Check if the left subtree may contain overlapping intervals
        if (node.Left != null && node.Left.MaxEnd.CompareTo(low) >= 0)
        {
            SearchOverlappingIntervals(node.Left, low, high, result);
        }

        // Check if the right subtree may contain overlapping intervals
        if (node.Right != null)
        {
            SearchOverlappingIntervals(node.Right, low, high, result);
        }
    }

    private void SearchIntervalsEndingBefore(IntervalTreeNode<IntervalType, PayloadType>? node, IntervalType high, List<Data<IntervalType, PayloadType>> result)
    {
        if (node == null)
            return;

        // Check if the interval overlaps with the current node's interval
        if (node.End.CompareTo(high) <= 0)
        {
            foreach (var payload in node.Payloads)
                result.Add(new Data<IntervalType, PayloadType>(node.Start, node.End, payload));
        }

        if (node.Left != null && node.Left.Start.CompareTo(high) <= 0)
            SearchIntervalsEndingBefore(node.Left, high, result);

        if (node.Right != null)
            SearchIntervalsEndingBefore(node.Right, high, result);
    }

    private IntervalTreeNode<IntervalType, PayloadType>? RemoveNode(IntervalTreeNode<IntervalType, PayloadType>? node, IntervalType low, IntervalType high, PayloadType payload, bool forceDelete = false)
    {
        if (node == null)
            return null;

        if (low.CompareTo(node.Start) < 0)
        {
            node.Left = RemoveNode(node.Left, low, high, payload);
        }
        else if (low.CompareTo(node.Start) > 0)
        {
            node.Right = RemoveNode(node.Right, low, high, payload);
        }
        else if (high.CompareTo(node.End) == 0)
        {                     
            if (forceDelete)
            {
                node.Payloads.Clear();
            }
            else if (payload != null)
            {
                node.Payloads.Remove(payload);
            }

            if (node.Payloads.Count() == 0)
            {
                if (node.Left == null)
                {
                    return node.Right;
                }
                else if (node.Right == null)
                {
                    return node.Left;
                }

                // Node has two children, find the in-order successor
                var succ = node.Right;
                while (succ.Left != null)
                    succ = succ.Left;

                node.Start = succ.Start;
                node.End = succ.End;
                node.Payloads = succ.Payloads;
                node.MaxEnd = node.Left.MaxEnd.CompareTo(node.Right.MaxEnd) > 0 ? node.Left.MaxEnd : node.Right.MaxEnd;

                node.Right = RemoveNode(node.Right, succ.Start, succ.End, payload, true);
            }
        }

        return node;
    }
}


