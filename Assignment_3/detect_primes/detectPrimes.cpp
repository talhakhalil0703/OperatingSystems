#include "detectPrimes.h"
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <atomic>
#include <thread>
#include <iostream>
#include <mutex>
#include <condition_variable>

//Simple Debug macro 
#if 0
    #define DEBUG(x) do { std::cerr << #x << ": " << x << std::endl; } while (0)
# else 
    #define DEBUG(x) 
#endif


static std::atomic<bool> finished(false);
static std::atomic<bool> current_is_prime(false);
static std::atomic<bool> got_answer(false);

//Pavol's Barrier Code
class simple_barrier {

    std::mutex m_;
    std::condition_variable cv_;
    int n_remaining_, count_;
    bool coin_;

public:

    simple_barrier(int count) {
        count_ = count;
        n_remaining_ = count_;
        coin_ = false;
    }

    bool wait() {
        std::unique_lock<std::mutex> lk(m_);
        if (n_remaining_ == 1) {
            coin_ = !coin_;
            n_remaining_ = count_;
            cv_.notify_all();
            return true;
        }
        auto old_coin = coin_;
        n_remaining_--;
        cv_.wait(lk, [&]() { return old_coin != coin_; });
        return false;
    }

};

void threaded_prime_calc(const std::vector<int64_t>& nums, 
                        std::vector<int64_t>& results, 
                        uint64_t& index, 
                        simple_barrier & barrier, 
                        int number_of_threads,
                        int thread_number,
                        int64_t& max,
                        int64_t& value_to_check_for_prime)
{
    while (true) {
        //Pick thread for serial task
        if (barrier.wait()) {
            //Serial code
            got_answer = false;
            // if no more numbers left leave
            if (index == nums.size()) {
                // Reached the end
                finished = true;
            } else {
                value_to_check_for_prime = nums[index++];
                //If simple answer compute here
                if (value_to_check_for_prime < 2) got_answer = true;
                else if (value_to_check_for_prime <= 3) {
                    got_answer = true;
                    results.push_back(value_to_check_for_prime);
                } else if (value_to_check_for_prime % 2 == 0) got_answer = true; // handle multiples of 2
                else if (value_to_check_for_prime % 3== 0) got_answer = true; // handle multiples of 3
            }
            
            if (!got_answer) {
                //If not a simple answer setup work for threading
                DEBUG(value_to_check_for_prime);
                int64_t sq = value_to_check_for_prime;
                max = sqrt(sq);
                DEBUG(max);
                //If we still need to calculate....
                current_is_prime = true;
            }
        }
  
        barrier.wait();
        
        if (finished) { // If we finished all the numbers we should leave every thread
            return;
        }
        
        // If we have the answer already we need to get all thread onto the next number
        if (got_answer) {
            DEBUG(got_answer);
            continue;
        }       

        int64_t i = 5;
        //Checking if its prime using a uinque divisor seeded by the thread index
        while (true) {
            //If any thread finds its not a prime lets leave 
            if (!current_is_prime || (i + (thread_number * 6) > max && i + 2 + (thread_number * 6) > max)) {
                break;
            }
  
            //We only write if either of the statement is true, otherwise we don't make the atomic call
            if (value_to_check_for_prime % (i + (thread_number * 6)) == 0) current_is_prime = false;
            else if (value_to_check_for_prime % (i + 2 + (thread_number * 6)) == 0) current_is_prime = false;
            i += (6 * number_of_threads);
        }

        //Serial task, storing the value if it was a prime
        if (barrier.wait()) {
            if (current_is_prime) results.push_back(value_to_check_for_prime);
        }
        barrier.wait();   
    }
}

std::vector<int64_t>
detect_primes(const std::vector<int64_t>& nums, int n_threads)
{
    std::vector<int64_t> result;
    std::thread threads[n_threads];
    simple_barrier barrier(n_threads);
    uint64_t index = 0;
    int64_t max = 0;
    int64_t value_to_check_for_prime = 0;
    
    for (int i = 0; i < n_threads; i++)
    {
        threads[i] = std::thread(threaded_prime_calc, 
                                std::ref(nums), 
                                std::ref(result), 
                                std::ref(index),   
                                std::ref(barrier), 
                                n_threads, 
                                i,
                                std::ref(max),
                                std::ref(value_to_check_for_prime));
    }

    for (auto& t : threads)
    {
        t.join();
    }
    return result;
}