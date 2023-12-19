
#include <gtest/gtest.h>

#include <iostream>
#include <vector>
#include <tuple>

typedef std::vector<std::tuple<int, int>> PathArray;

void printRoute(PathArray &path)
{
    std::vector<std::vector<char>> matrix;

    for (auto step : path)
    {
        auto x = std::get<0>(step);
        auto y = std::get<1>(step);

        if (y >= matrix.size())
        {
            matrix.resize(y + 1);

            for (auto row = 0; row < y; row++)
                matrix[row].resize(y + 1);
        }

        if (x >= matrix[y].size())
            matrix[y].resize(x + 1);

        matrix[y][x] = 'x';
    }

    for (auto row = 0; row < matrix.size(); row++)
    {
        for (auto col = 0; col < matrix[row].size(); col++)
        {
            if (matrix[row][col] == 'x')
                std::cout << 'X';
            else
                std::cout << ' ';
        }

        std::cout << std::endl;
    }

    std::cout << std::endl;
}

bool routes(int x, int y, PathArray &path)
{
    if (x == 0 && y == 0)
    {
        printRoute(path);
        return true;
    }

    auto success = false;

    if (x > 0)
    {
        path.push_back({x - 1, y});
        success = routes(x - 1, y, path);
        path.pop_back();
    }

    if (y > 0)
    {
        path.push_back({x, y - 1});
        success = routes(x, y - 1, path);
        path.pop_back();
    }

    return success;
}

TEST(BackTrack, run)
{
    PathArray path;

    path.push_back({3, 3});
    routes(3, 3, path);
}