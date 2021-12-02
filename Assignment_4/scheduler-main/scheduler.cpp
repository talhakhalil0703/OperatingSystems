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

void simulate_rr(
    int64_t quantum, int64_t max_seq_len, std::vector<Process> & processes, std::vector<int> & seq)
{
    int64_t cpu_time = 0;
    int64_t curr_process_bursts = 0;
    std::vector<int> rq, jq;
    std::vector<int64_t> remaining_bursts;
    seq.clear();

    int jobs_remaining = processes.size();
    int new_process_id = 0;
    int iteration_count = 0;
    while (1) {
        // if jobs_remaining == 0 break
        if (jobs_remaining == 0) break;

        DEBUG(iteration_count++);
        DEBUG(cpu_time);
        DEBUG(rq.size());
        DEBUG(cpu);
        DEBUG(curr_process_bursts);

        // if process in cpu is done
        //     mark process done
        //     set CPU idle
        //     jobs_remaining --
        //     continue
        if (cpu != -1) {
            if (remaining_bursts[cpu] == 0) {
                DEBUG("Process in cpu is done");
                processes[cpu].finish_time = cpu_time;
                cpu = -1;
                jobs_remaining -= 1;
                curr_process_bursts = 0;
                continue;
            }
        }

        // Insert existing process into queue before adding new process if they are the same time
        // If exceed time slice switch out the process
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
            continue;
        }

        // if a new process arriving
        //     add new process to RQ
        //     continue
        if (new_process_id < processes.size()
            && processes[new_process_id].arrival_time <= cpu_time) {
            DEBUG("New process is arriving");
            DEBUG(new_process_id);
            rq.push_back(processes[new_process_id].id);
            remaining_bursts.push_back(processes[new_process_id].burst);
            new_process_id++;
            continue;
        }

        // if cpu is idle and RQ not empty
        //     move process from RQ to CPU
        //     continue
        if (cpu == -1 && rq.size() > 0) {
            DEBUG("CPU is idle and RQ not empty");
            cpu = rq.front();
            rq.erase(rq.begin());
            DEBUG(cpu);
            // If first time starting the process note it's start time
            if (processes[cpu].start_time == -1) processes[cpu].start_time = cpu_time;
            if (seq.back() != cpu) seq.push_back(cpu);
            continue;
        }

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