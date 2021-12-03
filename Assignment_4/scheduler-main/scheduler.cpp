// you need to modify this file

#include "scheduler.h"
#include "common.h"
#include <cmath>
#include <iostream>

int64_t remaining_slice;
int cpu = -1;

int64_t cpu_time = 0;
int64_t curr_process_bursts = 0;
std::vector<int> rq, jq;
std::vector<int64_t> remaining_bursts;
int jobs_remaining;
uint64_t new_process_id = 0;
int iteration_count = 0;

bool new_process_arrive(std::vector<Process> & processes)
{

    if (new_process_id < processes.size() && processes[new_process_id].arrival_time <= cpu_time) {
        rq.push_back(processes[new_process_id].id);
        remaining_bursts.push_back(processes[new_process_id].burst);
        new_process_id++;
        return true;
    }
    return false;
}

bool while_new_process_arrive(std::vector<Process> & processes)
{
    bool added_new = false;

    while (new_process_id < processes.size() && processes[new_process_id].arrival_time < cpu_time) {
        rq.push_back(processes[new_process_id].id);
        remaining_bursts.push_back(processes[new_process_id].burst);
        new_process_id++;
        added_new = true;
    }
    return added_new;
}

bool current_job_is_done(std::vector<Process> & processes)
{
    if (cpu != -1) {
        if (remaining_bursts[cpu] <= 0) {
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
        while_new_process_arrive(processes);
        rq.push_back(cpu);
        if (rq.size() > 0) {
            cpu = -1;
            curr_process_bursts = 0;
        }
        return true;
    }
    return false;
}

bool cpu_idle_and_job_available(std::vector<Process> & processes, std::vector<int> & seq)
{
    if (cpu == -1 && rq.size() > 0) {
        cpu = rq.front();
        rq.erase(rq.begin());
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
        if (jobs_remaining == 0) break;

        if (current_job_is_done(processes)) continue;
        if (time_slice_exceeded(processes, quantum)) continue;
        if (new_process_arrive(processes)) continue;
        if (cpu_idle_and_job_available(processes, seq)) continue;

        if (cpu == -1) { // If idling note that.
            if (seq.back() != -1) seq.push_back(cpu);
            if (rq.size() == 0 && new_process_id < processes.size()) {
                cpu_time = processes[new_process_id].arrival_time;
            } else {
                cpu_time += 1;
            }
        } else { // If not idling then note that.
            int64_t time_skip = 0;
            if (jobs_remaining == 1) {
                time_skip = remaining_bursts[cpu];
            } else if (rq.size() == 0 && new_process_id < processes.size()) {
                time_skip = processes[new_process_id].arrival_time - cpu_time;
                if (time_skip > remaining_bursts[cpu]) {
                    time_skip = remaining_bursts[cpu];
                } else if (time_skip % quantum != 0) {
                    // Take quantum time skips
                    double x = std::ceil(1.0 * time_skip / quantum);
                    time_skip = x * quantum;
                }

            } else {
                time_skip = (remaining_bursts[cpu] >= quantum) ? quantum : remaining_bursts[cpu];
            }
            remaining_bursts[cpu] -= time_skip;
            curr_process_bursts += time_skip;
            cpu_time += time_skip;
        }
    }
}