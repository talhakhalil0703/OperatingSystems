// you need to modify this file

#include "scheduler.h"
#include "common.h"
#include <iostream>

#if 1
#define DEBUG(x)                                                                                   \
    do {                                                                                           \
        std::cerr << #x << ": " << x << std::endl;                                                 \
    } while (0)
#else
#define DEBUG(x)
#endif

int64_t remaining_slice;
int cpu = -1;

int64_t cpu_time = 0;
int64_t curr_process_bursts = 0;
std::vector<int> rq, jq;
std::vector<int64_t> remaining_bursts;
int jobs_remaining;
int new_process_id = 0;
int iteration_count = 0;

bool new_process_arrive(std::vector<Process> & processes)
{
    if (new_process_id < processes.size() && processes[new_process_id].arrival_time <= cpu_time) {
        DEBUG("New process is arriving");
        DEBUG(new_process_id);
        rq.push_back(processes[new_process_id].id);
        remaining_bursts.push_back(processes[new_process_id].burst);
        new_process_id++;
        return true;
    }
    return false;
}

bool current_job_is_done(std::vector<Process> & processes)
{
    if (cpu != -1) {
        if (remaining_bursts[cpu] == 0) {
            DEBUG("Process in cpu is done");
            processes[cpu].finish_time = cpu_time;
            cpu = -1;
            jobs_remaining -= 1;
            curr_process_bursts = 0;
            return true;
        }
    }
    return false;
}

bool time_slice_exceeded(std::vector<Process> & processes, int64_t quantum)
{
    if (curr_process_bursts >= quantum) {
        DEBUG("Exceeded time slice");
        DEBUG(cpu);
        rq.push_back(cpu);
        if (rq.size() > 0) {
            cpu = -1;
            curr_process_bursts = 0;
        }
        DEBUG("Next process is");
        DEBUG(rq.front());
        return true;
    }
    return false;
}

bool cpu_idle_and_job_available(std::vector<Process> & processes, std::vector<int> & seq)
{
    if (cpu == -1 && rq.size() > 0) {
        DEBUG("CPU is idle and RQ not empty");
        cpu = rq.front();
        rq.erase(rq.begin());
        DEBUG(cpu);
        // If first time starting the process note it's start time
        if (processes[cpu].start_time == -1) processes[cpu].start_time = cpu_time;
        if (seq.back() != cpu) seq.push_back(cpu);
        return true;
    }
    return false;
}

void simulate_rr(
    int64_t quantum, int64_t max_seq_len, std::vector<Process> & processes, std::vector<int> & seq)
{
    seq.clear();
    jobs_remaining = processes.size();

    while (1) {
        // if jobs_remaining == 0 break
        if (jobs_remaining == 0) break;

        DEBUG(iteration_count++);
        DEBUG(cpu_time);
        DEBUG(rq.size());
        DEBUG(cpu);
        DEBUG(curr_process_bursts);

        if (current_job_is_done(processes)) continue;
        if (time_slice_exceeded(processes, quantum)) continue;
        if (new_process_arrive(processes)) continue;

        // if cpu is idle and RQ not empty
        //     move process from RQ to CPU
        //     continue
        if (cpu_idle_and_job_available(processes, seq)) continue;
        // execute one burst of job on CPU, or stay idle
        DEBUG("Executing burst on cpu or idling");
        DEBUG(cpu);
        if (cpu == -1) { // If idling note that.
            if (seq.back() != -1) seq.push_back(cpu);
        } else { // If not idling then note that.
            remaining_bursts[cpu] -= 1;
            curr_process_bursts++;
        }
        cpu_time += 1;
    }
}