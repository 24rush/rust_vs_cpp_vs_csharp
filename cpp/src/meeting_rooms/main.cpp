#include <iostream>
#include <vector>
#include <charconv>

#include "include/meeting_rooms.h"

void generate_indices(int low, int high, std::vector<int> &out)
{
    if (low > high)
        return;

    int mid = (low + high) / 2;
    out.push_back(mid);

    generate_indices(low, mid - 1, out);
    generate_indices(mid + 1, high, out);
}

int main(int argc, char *argv[])
{
    size_t nr_threads = 1, nr_intervals = 5000000, nr_rooms = 3;

    if (argc > 1 && (argc - 1) % 2 == 0)
    {
        const std::vector<std::string_view> args(argv, argv + argc);

        auto next_token = [&args](int pos)
        {
            auto str = args[pos + 1];
            auto result = -1;
            std::from_chars(str.data(), str.data() + str.size(), result);

            return result;
        };
        
        for (auto i = 0; i < argc; i++)
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

    std::cout << "Config: " << nr_threads << " threads | " << nr_rooms << " rooms | " << nr_intervals << " intervals\n";

    MeetingRoomScheduler scheduler;

    for (size_t i = 0; i < nr_rooms; i++)
    {
        auto mr = MeetingRoom{"#M" + std::to_string(i), i};
        scheduler.registerRoom(mr);
    }

    std::vector<int> indices;
    generate_indices(1, nr_intervals, indices);

    auto requestRoom = [&scheduler, &indices]()
    {
        auto now = system_clock::now();

        for (size_t i = 0; i < indices.size(); i++)
        {
            auto rnd_ts = DateTimeSlot((now + seconds(indices[i])), 1);

            auto room = scheduler.requestRoom(rnd_ts);
        }
    };

    std::vector<std::thread> threads;
    for (size_t tn = 0; tn < nr_threads; tn++)
    {
        threads.push_back(std::thread{requestRoom});
    }

    for (auto &thread : threads)
    {
        thread.join();
    }

    return 0;
}
