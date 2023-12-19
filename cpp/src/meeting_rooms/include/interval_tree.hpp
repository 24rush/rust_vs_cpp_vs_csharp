#pragma once

#include <memory>
#include <mutex>
#include <shared_mutex>

#include <set>
#include <list>

template <typename IntervalType, typename PayloadType>
class IntervalTree
{
public:
    // Structure to hold data that clients will pass (low, high, <payload>)
    struct Data
    {
        IntervalType low;
        IntervalType high;

        PayloadType payload;
    };

    void insert(Data iData)
    {
        std::unique_lock lock(this->rootSync);
        this->root = this->insertInternal(std::move(this->root), iData);
    }

    void remove(Data iData)
    {
        std::unique_lock lock(this->rootSync);
        this->removeInternal(this->root, iData.low, iData.high, iData.payload);
    }

    std::list<Data> getOverlappingIntervalsWith(const IntervalType &low, const IntervalType &high)
    {
        std::list<Data> overlappingIntervals;

        {
            std::shared_lock lock(this->rootSync);
            this->searchOverlappingIntervals(this->root, std::min(low, high), std::max(low, high), overlappingIntervals);
        }

        return overlappingIntervals;
    }

    std::list<Data> getIntervalsEndingBefore(const IntervalType &high)
    {
        std::list<Data> intervalsEndingBefore;

        {
            std::shared_lock lock(this->rootSync);
            this->searchIntervalsEndingBefore(this->root, high, intervalsEndingBefore);
        }

        return intervalsEndingBefore;
    }

    bool isEmpty()
    {
        std::unique_lock lock(this->rootSync);
        auto empty = this->root == nullptr;

        return empty;
    }

protected:
    // The interval node type in the interval tree
    struct IntervalTreeNode
    {
        // Merges payloads of equal (low, high) intervals
        std::set<PayloadType> payloads;

        IntervalType maxHigh;
        IntervalType low;
        IntervalType high;

        std::unique_ptr<IntervalTreeNode> left;
        std::unique_ptr<IntervalTreeNode> right;
    };

    using IntervalTreeNodePtr = std::unique_ptr<IntervalTreeNode>;

    IntervalTreeNodePtr root;
    mutable std::shared_mutex rootSync;

    IntervalTreeNodePtr insertInternal(IntervalTreeNodePtr root, const Data &data)
    {
        auto [low, high, payload] = data;

        if (root == nullptr)
            return std::make_unique<IntervalTreeNode>(IntervalTreeNode{{payload}, high, low, high, 0, 0});

        if (root->low == low && root->high == high)
        {
            root->payloads.insert(payload);
            return root;
        }

        // If root's low value is smaller, then new interval goes to left subtree
        if (low < root->low)
            root->left = insertInternal(std::move(root->left), data);
        else
            root->right = insertInternal(std::move(root->right), data);

        // Update the maxHigh value of this ancestor if needed
        root->maxHigh = std::max(root->maxHigh, high);

        return root;
    }

    void removeInternal(IntervalTreeNodePtr &root, const IntervalType &low, const IntervalType &high, PayloadType payload)
    {
        if (root == nullptr)
            return;

        if (root->low == low && root->high == high)
        {
            root->payloads.erase(payload);

            if (root->payloads.size() == 0)
            {
                std::optional<IntervalType> childMax;
                if (root->left != nullptr && root->right != nullptr)
                    childMax = std::max(root->left->maxHigh, root->right->maxHigh);
                else if (root->left != nullptr)
                    childMax = root->left->maxHigh;
                else if (root->right != nullptr)
                    childMax = root->right->maxHigh;

                root = this->removeNode(std::move(root), low, high);

                if (root != nullptr)
                    root->maxHigh = childMax.has_value() ? childMax.value() : root->high;
            }

            return;
        }

        // If the left child exists and its maximum high value is greater than or equal to the low value of the interval,
        // there may be an overlapping node in the left subtree.
        if (root->left && root->left->high >= low)
            removeInternal(root->left, low, high, payload);
        else
            removeInternal(root->right, low, high, payload);
    }

    IntervalTreeNodePtr removeNode(IntervalTreeNodePtr root, const IntervalType &low, const IntervalType &high)
    {
        if (root == nullptr)
            return nullptr;

        // Find the node to delete
        if (low < root->low)
            root->left = removeNode(std::move(root->left), low, high);
        else if (low > root->low)
            root->right = removeNode(std::move(root->right), low, high);
        else if (root->low == low && root->high == high)
        {
            // Node with only one child or no child
            if (root->left == nullptr)
                return std::move(root->right);
            else if (root->right == nullptr)
                return std::move(root->left);

            // Node with two children, get the inorder successor
            IntervalTreeNode *temp = root->right.get();

            while (temp->left != nullptr)
                temp = temp->left.get();

            // Copy the inorder successor's content to this node
            root->low = temp->low;
            root->high = temp->high;

            // Delete the inorder successor
            root->right = removeNode(std::move(root->right), temp->low, temp->high);
        }

        return root;
    }

    void searchOverlappingIntervals(IntervalTreeNodePtr &root, const IntervalType &low, const IntervalType &high, std::list<Data> &overlaps)
    {
        if (root == nullptr)
            return;

        // If given interval overlaps with root
        if (low < root->high && high > root->low)
        {
            for (auto payload : root->payloads)
                overlaps.push_back(Data{root->low, root->high, payload});
        }

        // If left child of root is present and maxHigh of left child is
        // greater than or equal to given interval, then i may
        // overlap with an interval in left subtree
        if (root->left != nullptr && root->left->maxHigh >= low)
            searchOverlappingIntervals(root->left, low, high, overlaps);

        // interval can only overlap with right subtree
        if (root->right != nullptr)
            searchOverlappingIntervals(root->right, low, high, overlaps);
    }

    void searchIntervalsEndingBefore(IntervalTreeNodePtr &root, const IntervalType &high, std::list<Data> &overlaps)
    {
        if (root == nullptr)
            return;

        // If given interval ends before high
        if (root->high <= high)
        {
            for (auto payload : root->payloads)
                overlaps.push_back(Data{root->low, root->high, payload});
        }

        if (root->left != nullptr && root->left->low <= high)
            searchIntervalsEndingBefore(root->left, high, overlaps);

        if (root->right != nullptr)
            searchIntervalsEndingBefore(root->right, high, overlaps);
    }
};