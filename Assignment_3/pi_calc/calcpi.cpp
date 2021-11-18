#include "calcpi.h"
#include <thread>
#include <iostream>

struct Task
{
    uint64_t start_x, end_x;
    uint64_t partial_count;
};

void inner_loop_count(int r, double rsq, Task& task)
{
    uint64_t count = 0;
    for (double x = task.start_x; x < task.end_x; x++)
    {
        for (double y = 0; y <= r; y++)
        {
            if (x * x + y * y <= rsq)
            {
                count++;
            }
        }
    }
    task.partial_count = count;
    return;
}

uint64_t count_pixels(int r, int n_threads)
{
    Task tasks[n_threads];
    double rsq = double(r) * r;
    uint64_t count = 0;
    uint64_t incremental_r = r / n_threads;
    int remainder_r = r % n_threads;
    uint64_t index[n_threads + 1];
    //End value, we do up to not including last value
    index[n_threads] = r + 1;

    for (int i = 0; i < n_threads; i++)
    {
        index[i] = incremental_r * i;
    }
    index[n_threads] = r;
    
    for (int i = 0; i < remainder_r; i++)
    {
        index[i] = index[i] + 1;
        index[i + 1] = index[i + 1] + 1;
    }
    index[0] = 1;

    std::thread threads[n_threads];
    for (int i = 0; i < n_threads; i++)
    {
        tasks[i].start_x = index[i];
        tasks[i].end_x = index[i + 1];
        tasks[i].partial_count = 0;
        threads[i] = std::thread(inner_loop_count, r, rsq, std::ref(tasks[i]));
    }

    for (auto& t : threads)
    {
        t.join();
    }

    for (auto& task : tasks)
    {
        count += task.partial_count;
    }

    return count * 4 + 1;
}