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

bool all_processes_in_rq_have_start_time(std::vector<Process> & processes)
{
    for (int64_t id : rq) {
        if (processes[id].start_time == -1) return false;
    }
    return true;
}

bool check_for_idle_time_skip(
    std::vector<Process> & processes, std::vector<int> & seq, int64_t max_seq_len, int64_t quantum)
{
    bool do_it = true;

    int64_t smallest_burst_finish = INT64_MAX;
    int64_t cpu_id = -1;
    for (int64_t id : rq) {
        if (remaining_bursts[id] < quantum) { do_it = false; }
        if (remaining_bursts[id] < smallest_burst_finish) {
            smallest_burst_finish = remaining_bursts[id];
            cpu_id = id;
        }
    }

    bool process = false;
    if (new_process_id < processes.size()) {

        if ((rq.size() + 1) * quantum < (processes[new_process_id].arrival_time - cpu_time)) {
            do_it = true;
            process = true;
            if ((processes[new_process_id].arrival_time - cpu_time) < smallest_burst_finish) {
                smallest_burst_finish = processes[new_process_id].arrival_time - cpu_time;
            }
        } else {
            do_it = false;
        }
    }

    // Shortest time recieved

    // int64_t N = std::floor(1.0 * smallest_burst_finish / quantum);
    int64_t N = (process) ? (smallest_burst_finish / (quantum * rq.size())) - 1
                          : (smallest_burst_finish / (quantum)) - 1;

    if (N <= 0) do_it = false;

    if (do_it) {
        // Go up until loop before and modify everything
        int64_t time_spent = 0;
        for (int64_t id : rq) {
            remaining_bursts[id] -= N * quantum;
            time_spent += N * quantum;
        }
        cpu_time += time_spent;
        curr_process_bursts = 0;

        for (int i = 0; i < N; i++) {
            for (int64_t id : rq) {
                if (seq.size() >= max_seq_len) return true;
                if (seq.back() != id) {
                    seq.push_back(id);
                } else {
                    if (rq.size() == 1) return true;
                }
            }
        }

        cpu = rq.front();
        rq.erase(rq.begin());
        return true;
    }
    return false;
}

bool cpu_idle_and_job_available(
    std::vector<Process> & processes, std::vector<int> & seq, int64_t quantum, int64_t max_seq_len)
{
    if (cpu == -1 && rq.size() > 0) {
        if (all_processes_in_rq_have_start_time(processes)) {
            if (check_for_idle_time_skip(processes, seq, max_seq_len, quantum)) return true;

            cpu = rq.front();
            rq.erase(rq.begin());
            // If first time starting the process note it's start time
            if (processes[cpu].start_time == -1) processes[cpu].start_time = cpu_time;
            if (seq.back() != cpu && seq.size() < max_seq_len) seq.push_back(cpu);
        } else {
            cpu = rq.front();
            rq.erase(rq.begin());
            // If first time starting the process note it's start time
            if (processes[cpu].start_time == -1) processes[cpu].start_time = cpu_time;
            if (seq.back() != cpu && seq.size() < max_seq_len) seq.push_back(cpu);
        }

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
        if (cpu_idle_and_job_available(processes, seq, quantum, max_seq_len)) continue;

        if (cpu == -1) { // If idling note that.
            if (seq.back() != -1 && seq.size() < max_seq_len) seq.push_back(cpu);
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