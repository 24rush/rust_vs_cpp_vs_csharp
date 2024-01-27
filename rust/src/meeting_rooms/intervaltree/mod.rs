use std::collections::HashSet;

// Structure to hold data that clients will pass (low, high, <payload>)
#[derive(Debug, Clone)]
pub struct Data<
    IntervalType: std::cmp::PartialEq + std::cmp::PartialOrd,
    PayloadType: std::cmp::Eq + std::hash::Hash,
> {
    pub low: IntervalType,
    pub high: IntervalType,

    pub payload: PayloadType,
}

impl<
        IntervalType: std::cmp::PartialEq + std::cmp::PartialOrd,
        PayloadType: std::cmp::Eq + std::hash::Hash + Clone,
    > Data<IntervalType, PayloadType>
{
    pub fn new(low: IntervalType, high: IntervalType, payload: &PayloadType) -> Self {
        Data {
            low: low,
            high: high,
            payload: (*payload).clone(),
        }
    }
}

type IntervalTreeNodeBox<IntervalType, PayloadType> =
    Box<IntervalTreeNode<IntervalType, PayloadType>>;

// The interval node type in the interval tree
#[derive(Debug, Clone)]
struct IntervalTreeNode<IntervalType, PayloadType> {
    // Merges payloads of equal (low, high) intervals
    payloads: HashSet<PayloadType>,

    max_high: IntervalType,
    low: IntervalType,
    high: IntervalType,

    left: Option<IntervalTreeNodeBox<IntervalType, PayloadType>>,
    right: Option<IntervalTreeNodeBox<IntervalType, PayloadType>>,
}

impl<
        IntervalType: std::cmp::PartialEq + std::cmp::PartialOrd + std::cmp::Ord,
        PayloadType: std::cmp::Eq + std::hash::Hash,
    > IntervalTreeNode<IntervalType, PayloadType>
where
    IntervalType: Copy,
    PayloadType: Clone,
{
    fn new(data: Data<IntervalType, PayloadType>) -> Self {
        IntervalTreeNode {
            payloads: HashSet::from([data.payload]),
            low: data.low,
            high: data.high,
            max_high: data.high,

            left: None,
            right: None,
        }
    }

    // Insert a new interval into the tree
    fn insert(&mut self, data: Data<IntervalType, PayloadType>) {
        if data.low < self.low {
            if let Some(left) = &mut self.left {
                left.insert(data);
            } else {
                self.left = Some(Box::new(IntervalTreeNode::new(data)));
            }
        } else {
            if let Some(right) = &mut self.right {
                right.insert(data);
            } else {
                self.right = Some(Box::new(IntervalTreeNode::new(data)));
            }
        }
    }

    // Removes node defined by (low, high, payload), if payload is None then it removes the completely
    fn remove(
        &mut self,
        low: IntervalType,
        high: IntervalType,
        payload: Option<PayloadType>,
    ) -> Option<Box<Self>> {
        if low < self.low {
            if let Some(mut left) = self.left.take() {
                self.left = left.remove(low, high, payload);
            }
        } else if low > self.low {
            if let Some(mut right) = self.right.take() {
                self.right = right.remove(low, high, payload);
            }
        } else {
            if self.high == high {
                if payload.is_some() {
                    self.payloads.remove(&payload.unwrap());
                } else {
                    self.payloads.clear();
                }

                if self.payloads.is_empty() {
                    if self.left.is_none() {
                        return self.right.take();
                    } else if self.right.is_none() {
                        return self.left.take();
                    } else {
                        let mut successor = self.right.as_ref();
                        while let Some(inner) = successor.as_ref().and_then(|n| n.left.as_ref()) {
                            successor = Some(inner);
                        }

                        let succ_node = successor.unwrap();

                        self.low = succ_node.low;
                        self.high = succ_node.high;
                        self.payloads = succ_node.payloads.clone();
                        self.max_high = std::cmp::max(
                            self.left.as_ref().unwrap().max_high,
                            self.right.as_ref().unwrap().max_high,
                        );

                        self.right = self
                            .right
                            .take()
                            .and_then(|mut right| right.remove(self.low, self.high, None));
                    }
                }
            }
        }

        Some(Box::new(self.clone()))
    }

    // Get intervals that end before a given value
    fn get_intervals_ending_before(
        &self,
        end_value: IntervalType,
        result: &mut Vec<Data<IntervalType, PayloadType>>,
    ) {
        if self.high <= end_value {
            for payload in &self.payloads {
                result.push(Data::new(self.low, self.high, payload));
            }
        }

        if let Some(left) = &self.left {
            if left.low <= end_value {
                left.get_intervals_ending_before(end_value, result);
            }
        }

        if let Some(right) = &self.right {
            right.get_intervals_ending_before(end_value, result);
        }
    }

    // Get intervals that overlap with (low, high)
    fn get_intervals_overlapping_with(
        &self,
        low: IntervalType,
        high: IntervalType,
        result: &mut Vec<Data<IntervalType, PayloadType>>,
    ) {
        if low < self.high && high > self.low {
            for payload in &self.payloads {
                result.push(Data::new(self.low, self.high, payload));
            }
        }

        // If left child of root is present and maxHigh of left child is
        // greater than or equal to given interval, then i may
        // overlap with an interval in left subtree
        if let Some(left) = &self.left {
            if left.max_high >= low {
                left.get_intervals_overlapping_with(low, high, result);
            }
        }

        if let Some(right) = &self.right {
            right.get_intervals_overlapping_with(low, high, result);
        }
    }
}

// Define an IntervalTree to wrap the root TreeNode
#[derive(Debug, Default)]
pub struct IntervalTree<IntervalType, PayloadType> {
    root: Option<IntervalTreeNodeBox<IntervalType, PayloadType>>,
}

impl<
        IntervalType: std::cmp::PartialEq + std::cmp::PartialOrd + std::cmp::Ord,
        PayloadType: std::cmp::Eq + std::hash::Hash,
    > IntervalTree<IntervalType, PayloadType>
where
    IntervalType: Copy,
    PayloadType: Clone,
{
    pub fn new() -> Self {
        IntervalTree { root: None }
    }

    pub fn is_empty(&self) -> bool {
        self.root.is_none()
    }

    pub fn insert(&mut self, data: Data<IntervalType, PayloadType>) {
        if let Some(root) = &mut self.root {
            root.insert(data);
        } else {
            self.root = Some(Box::new(IntervalTreeNode::new(data)));
        }
    }

    pub fn remove(&mut self, data: &Data<IntervalType, PayloadType>) {
        if self.root.is_some() {
            self.root = self
                .root
                .take()
                .and_then(|mut root| root.remove(data.low, data.high, Some(data.payload.clone())));
        }
    }

    pub fn get_intervals_ending_before(
        &self,
        end_value: IntervalType,
    ) -> Vec<Data<IntervalType, PayloadType>> {
        let mut result = Vec::new();

        if let Some(root) = &self.root {
            root.get_intervals_ending_before(end_value, &mut result);
        }

        result
    }

    pub fn get_overlapping_intervals_with(
        &self,
        low: IntervalType,
        high: IntervalType,
    ) -> Vec<Data<IntervalType, PayloadType>> {
        let mut result = Vec::new();

        if let Some(root) = &self.root {
            root.get_intervals_overlapping_with(
                std::cmp::min(low, high),
                std::cmp::max(low, high),
                &mut result,
            );
        }

        result
    }
}

#[cfg(test)]
mod tests {
    use crate::meeting_rooms::intervaltree::Data;

    //#[test]
    fn insert() {
        // Create an interval tree
        let mut interval_tree = crate::meeting_rooms::intervaltree::IntervalTree::new();

        // Insert intervals into the tree
        interval_tree.insert(Data {
            low: 1,
            high: 5,
            payload: 1,
        });
        interval_tree.insert(Data {
            low: 3,
            high: 8,
            payload: 2,
        });
        interval_tree.insert(Data {
            low: 2,
            high: 8,
            payload: 2,
        });
        interval_tree.insert(Data {
            low: 6,
            high: 10,
            payload: 3,
        });
        interval_tree.insert(Data {
            low: 9,
            high: 12,
            payload: 2,
        });
        interval_tree.insert(Data {
            low: 11,
            high: 15,
            payload: 3,
        });

        let result = interval_tree.get_intervals_ending_before(10);
        assert!(result.len() == 4);

        let result = interval_tree.get_intervals_ending_before(11);
        assert!(result.len() == 4);

        let result = interval_tree.get_intervals_ending_before(12);
        assert!(result.len() == 5);

        interval_tree.remove(&Data::new(1, 5, &1));
        let result = interval_tree.get_intervals_ending_before(10);
        println!("{}", result.len());
        assert!(result.len() == 3);

        interval_tree.remove(&Data::new(3, 8, &3));
        let result = interval_tree.get_intervals_ending_before(10);
        assert!(result.len() == 3);

        interval_tree.remove(&Data::new(3, 8, &2));
        let result = interval_tree.get_intervals_ending_before(10);
        assert!(result.len() == 2);
    }

    // #[test]
    fn get_overlaps_with() {
        let mut interval_tree = crate::meeting_rooms::intervaltree::IntervalTree::new();

        let intervals: Vec<(i32, i32, i32)> = [
            (1, 20, 4),
            (10, 30, 5),
            (17, 19, 6),
            (5, 20, 7),
            (12, 15, 8),
            (30, 40, 9),
        ]
        .to_vec();

        for interval in &intervals {
            interval_tree.insert(Data::new(interval.0, interval.1, &interval.2));
        }

        let overlaps = interval_tree.get_overlapping_intervals_with(6, 7);

        assert!(overlaps.len() == 2);
    }

    // #[test]
    fn remove() {
        let mut interval_tree = crate::meeting_rooms::intervaltree::IntervalTree::new();

        let intervals: Vec<(i32, i32, i32)> = [
            (0, 1, 1),
            (0, 1, 2),
            (3, 7, 3),
            (2, 6, 4),
            (10, 15, 5),
            (5, 6, 6),
            (4, 100, 7),
        ]
        .to_vec();

        for interval in &intervals {
            interval_tree.insert(Data::new(interval.0, interval.1, &interval.2));
        }

        assert!(interval_tree.get_overlapping_intervals_with(0, 1).len() == 2);
        assert!(interval_tree.get_overlapping_intervals_with(2, 7).len() == 4);
        assert!(interval_tree.get_overlapping_intervals_with(1, 2).len() == 0);
        assert!(interval_tree.get_overlapping_intervals_with(1, 100).len() == 5);
        assert!(interval_tree.get_overlapping_intervals_with(5, 12).len() == 5);
        assert!(interval_tree.get_overlapping_intervals_with(101, 102).len() == 0);
        assert!(interval_tree.get_overlapping_intervals_with(6, 2).len() == 4);

        let mut interval = intervals[0];
        interval_tree.remove(&Data::new(interval.0, interval.1, &interval.2));
        assert!(interval_tree.get_overlapping_intervals_with(-1, 1).len() == 1);

        interval = intervals[3];
        interval_tree.remove(&Data::new(interval.0, interval.1, &interval.2));
        assert!(interval_tree.get_overlapping_intervals_with(2, 3).len() == 0);

        interval = intervals[1];
        interval_tree.remove(&Data::new(interval.0, interval.1, &interval.2));
        assert!(interval_tree.get_overlapping_intervals_with(-1, 1).len() == 0);
    }

    //#[test]
    fn ending_before() {
        let mut interval_tree = crate::meeting_rooms::intervaltree::IntervalTree::new();

        let intervals: Vec<(i32, i32, i32)> = [
            (0, 1, 1),
            (0, 1, 2),
            (3, 7, 3),
            (2, 6, 4),
            (10, 15, 5),
            (5, 6, 6),
            (4, 100, 7),
        ]
        .to_vec();

        for interval in &intervals {
            interval_tree.insert(Data::new(interval.0, interval.1, &interval.2));
        }

        assert!(interval_tree.get_intervals_ending_before(1).len() == 2);
        assert!(interval_tree.get_intervals_ending_before(2).len() == 2);
        assert!(interval_tree.get_intervals_ending_before(6).len() == 4);
        assert!(interval_tree.get_intervals_ending_before(15).len() == 6);
    }
}
