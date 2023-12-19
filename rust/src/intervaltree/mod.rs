use std::collections::HashSet;

// Structure to hold data that clients will pass (low, high, <payload>)
#[derive(Debug, Clone)]
struct Data<
    IntervalType: std::cmp::PartialEq + std::cmp::PartialOrd,
    PayloadType: std::cmp::Eq + std::hash::Hash,
> {
    low: IntervalType,
    high: IntervalType,

    payload: PayloadType,
}

impl<
        IntervalType: std::cmp::PartialEq + std::cmp::PartialOrd,
        PayloadType: std::cmp::Eq + std::hash::Hash,
    > Data<IntervalType, PayloadType>
{
    fn new(low: IntervalType, high: IntervalType, payload: PayloadType) -> Self {
        Data {
            low: low,
            high: high,
            payload: payload,
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
        IntervalType: std::cmp::PartialEq + std::cmp::PartialOrd,
        PayloadType: std::cmp::Eq + std::hash::Hash,
    > IntervalTreeNode<IntervalType, PayloadType>
where
    IntervalType: Copy,
    PayloadType: Copy,
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

    // Get intervals that end before a given value
    fn get_intervals_ending_before(
        &self,
        end_value: IntervalType,
        result: &mut Vec<Data<IntervalType, PayloadType>>,
    ) {
        if self.high < end_value {
            for payload in &self.payloads {
                result.push(Data::new(self.low, self.high, *payload));
            }
        }

        if let Some(left) = &self.left {
            left.get_intervals_ending_before(end_value, result);
        }

        if let Some(right) = &self.right {
            right.get_intervals_ending_before(end_value, result);
        }
    }
}

// Define an IntervalTree to wrap the root TreeNode
#[derive(Debug)]
struct IntervalTree<IntervalType, PayloadType> {
    root: Option<IntervalTreeNodeBox<IntervalType, PayloadType>>,
}

impl<
        IntervalType: std::cmp::PartialEq + std::cmp::PartialOrd,
        PayloadType: std::cmp::Eq + std::hash::Hash,
    > IntervalTree<IntervalType, PayloadType>
where
    IntervalType: Copy,
    PayloadType: Copy,
{
    fn new() -> Self {
        IntervalTree { root: None }
    }

    fn insert(&mut self, data: Data<IntervalType, PayloadType>) {
        if let Some(root) = &mut self.root {
            root.insert(data);
        } else {
            self.root = Some(Box::new(IntervalTreeNode::new(data)));
        }
    }

    fn get_intervals_ending_before(&self, end_value: IntervalType) -> Vec<Data<IntervalType, PayloadType>> {
        let mut result = Vec::new();

        if let Some(root) = &self.root {
            root.get_intervals_ending_before(end_value, &mut result);
        }

        result
    }
}

#[cfg(test)]
mod tests {
    use crate::intervaltree::Data;

    #[test]
    fn main() {
        // Create an interval tree
        let mut interval_tree = crate::intervaltree::IntervalTree::new();

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

        // Get intervals that end before a given value (e.g., 10)
        let result = interval_tree.get_intervals_ending_before(10);

        // Print the result
        println!("Intervals ending before 10: {:?}", result);
    }
}
