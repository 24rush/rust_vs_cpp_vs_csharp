#include <gtest/gtest.h>

#include "../include/interval_tree.hpp"

TEST(interval_tree, ctor)
{
    using IntervalTreeType = IntervalTree<int, int>;
    auto tree = std::make_unique<IntervalTreeType>();

    std::vector<IntervalTreeType::Data> intervals{{1, 20, 4}, {10, 30, 5}, {17, 19, 6}, {5, 20, 7}, {12, 15, 8}, {30, 40, 9}};

    for (auto e : intervals)
        tree->insert(e);

    auto overlaps = tree->getOverlappingIntervalsWith(6, 7);

    EXPECT_EQ(overlaps.size(), 2);

    for (auto o : overlaps)
        std::cout << "overlaps: " << o.low << "|" << o.high << std::endl;
}

TEST(interval_tree, insert)
{
    using IntervalTreeType = IntervalTree<int, int>;
    auto tree = std::make_unique<IntervalTreeType>();

    std::vector<IntervalTreeType::Data> intervals{{1, 5, 4}, {3, 7, 5}, {2, 6, 6}, {10, 15, 7}, {5, 6, 8}, {4, 100, 9}};

    for (auto e : intervals)
    {
        auto overlaps = tree->getOverlappingIntervalsWith(e.low, e.high);
        if (overlaps.size())
        {
            std::cout << e.low << "-" << e.high << " conflicts " << std::endl;

            for (auto o : overlaps)
                std::cout << "\t" << o.payload << " " << o.low << "-" << o.high << std::endl;
        }

        tree->insert(e);
    }
}

TEST(interval_tree, remove)
{
    using IntervalTreeType = IntervalTree<int, int>;
    auto tree = std::make_unique<IntervalTreeType>();
    /*
    |xxx|
    |xxx|
                |xxxxxxxxxxxxxxx|
            |xxxxxxxxxxxxxxx|
                                             |xxxxxxxxxxxxxxxxxxx|
                        |xxx|
                    |xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx|
    0   1   2   3   4   5   6   7   8   9   10                  15                          100
    */
    std::vector<IntervalTreeType::Data> intervals{{0, 1, 1}, {0, 1, 2}, {3, 7, 3}, {2, 6, 4}, {10, 15, 5}, {5, 6, 6}, {4, 100, 7}};
    for (auto e : intervals)
        tree->insert(e);

    EXPECT_EQ(tree->getOverlappingIntervalsWith(0, 1).size(), 2);
    EXPECT_EQ(tree->getOverlappingIntervalsWith(2, 7).size(), 4);
    EXPECT_EQ(tree->getOverlappingIntervalsWith(1, 2).size(), 0);
    EXPECT_EQ(tree->getOverlappingIntervalsWith(1, 100).size(), 5);
    EXPECT_EQ(tree->getOverlappingIntervalsWith(5.5, 12).size(), 5);
    EXPECT_EQ(tree->getOverlappingIntervalsWith(101, 102).size(), 0);
    EXPECT_EQ(tree->getOverlappingIntervalsWith(6, 2).size(), 4);

    tree->remove(intervals[0]);
    EXPECT_EQ(tree->getOverlappingIntervalsWith(-1, 1).size(), 1);

    tree->remove(intervals[3]);
    EXPECT_EQ(tree->getOverlappingIntervalsWith(2, 3).size(), 0);

    tree->remove(intervals[1]);
    EXPECT_EQ(tree->getOverlappingIntervalsWith(-1, 1).size(), 0);
}

TEST(interval_tree, getIntervalsEndingBefore)
{
    using IntervalTreeType = IntervalTree<int, int>;
    auto tree = std::make_unique<IntervalTreeType>();
    /*
    |xxx|
    |xxx|
                |xxxxxxxxxxxxxxx|
            |xxxxxxxxxxxxxxx|
                                             |xxxxxxxxxxxxxxxxxxx|
                        |xxx|
                    |xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx|
    0   1   2   3   4   5   6   7   8   9   10                  15                          100
    */
    std::vector<IntervalTreeType::Data> intervals{{0, 1, 1}, {0, 1, 2}, {3, 7, 3}, {2, 6, 4}, {10, 15, 5}, {5, 6, 6}, {4, 100, 7}};
    for (auto e : intervals)
        tree->insert(e);

    EXPECT_EQ(tree->getIntervalsEndingBefore(1).size(), 2);
    EXPECT_EQ(tree->getIntervalsEndingBefore(2).size(), 2);
    EXPECT_EQ(tree->getIntervalsEndingBefore(6).size(), 4);
    EXPECT_EQ(tree->getIntervalsEndingBefore(15).size(), 6);
}