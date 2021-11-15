#include "detectPrimes.h"
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <atomic>
#include <thread>
#include <iostream>
#include <mutex>
#include <condition_variable>

#if 0
    #define DEBUG(x) do { std::cerr << #x << ": " << x << std::endl; } while (0)
# else 
    #define DEBUG(x) 
#endif
// returns true if n is prime, otherwise returns false
// -----------------------------------------------------------------------------
// to get full credit for this assignment, you will need to adjust or even
// re-write the code in this function to make it multithreaded.
static std::atomic<bool> finished(false);
static int64_t value_to_check_for_prime = 0;
static std::atomic<bool> current_is_prime(false);
static std::atomic<bool> got_answer(false);

//class AtomicIndex {
//    std::atomic<int> index = { -1 };
//public:
//    int get() {
//        return index++;
//    }
//};

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

static bool is_prime(int64_t n)
{
    // handle trivial cases
    if (n < 2) return false;
    if (n <= 3) return true; // 2 and 3 are primes
    if (n % 2 == 0) return false; // handle multiples of 2
    if (n % 3 == 0) return false; // handle multiples of 3
    // try to divide n by every number 5 .. sqrt(n)
    int64_t i = 5;
    int64_t max = sqrt(n);
    while (i <= max) {
        if (n % i == 0) return false;
        if (n % (i + 2) == 0) return false;
        i += 6;
    }
    // didn't find any divisors, so it must be a prime
    return true;
}


//Even more efficient approach would be to parallelize the inner loop(the loop inside the
//    is_prime function).In this approach, all threads would test the same number for primality.If
//    you choose this approach, you need to give each thread a different portion of divisors to check.


//detectPrimes() :
//    prepare memory for each thread
//    initialize empty array result[] – this could be a global variable
//    set global_finished = false – make it atomic to be safe
//    start N threads, each runs thread_function() on its own memory
//    join N threads
//    return results
//    thread_function() :
//    repeat forever :
//serial task – pick one thread using barrier
//get the next number from nums[]
//if no more numbers left :
//set global_finished = true to indicate to all threads to quit
//otherwise :
//divide work for each thread
//parallel task – executed by all threads, via barrier
//if global_finished flag is set, exit thread
//otherwise do the work assigned aboveand record per - thread result
//serial task – pick one thread using barrier
//combine the per - thread results and update the result[] array if necessary

void threaded_prime_calc(const std::vector<int64_t>& nums, 
                        std::vector<int64_t>& results, 
                        long unsigned int& index, 
                        simple_barrier & barrier, 
                        int number_of_threads,
                        int thread_number) 
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
                //Divide work for threads here
                value_to_check_for_prime = nums[index++];
                DEBUG(value_to_check_for_prime);
                if (value_to_check_for_prime < 1000) {
                    if (is_prime(value_to_check_for_prime)) {
                        results.push_back(value_to_check_for_prime);
                    }
                    got_answer = true;
                }
            }
        }
        uint64_t max = sqrt(value_to_check_for_prime);
        DEBUG(max);
        //If we still need to calculate....
        current_is_prime = true;
        barrier.wait();
        if (got_answer) {
            DEBUG(got_answer);
            continue;
        }
        if (!finished) { 
            DEBUG(thread_number);
            DEBUG(number_of_threads);

        //All check its prime
            uint64_t i = 5;
            while (i <= max) {
                if (!current_is_prime) {
                    break;
                }

                if (value_to_check_for_prime % (i+(thread_number*6)) == 0) current_is_prime = false;
                else if (value_to_check_for_prime % (i + 2 + (thread_number*6)) == 0) current_is_prime = false;
                i += (6*number_of_threads);
            }

        //Serial task
        //Store if prime
            if (barrier.wait()) {
                if (current_is_prime) results.push_back(value_to_check_for_prime);
            }
            barrier.wait();
        } else {
            return;
        }
        
    }
}

std::vector<int64_t>
detect_primes(const std::vector<int64_t>& nums, int n_threads)
{
    std::vector<int64_t> result;
    std::thread threads[n_threads];
    long unsigned int index = 0;
    simple_barrier barrier(n_threads);

    for (int i = 0; i < n_threads; i++)
    {
        threads[i] = std::thread(threaded_prime_calc, std::ref(nums), std::ref(result), std::ref(index), std::ref(barrier), n_threads, i);
    }

    for (auto& t : threads)
    {
        t.join();
    }
    return result;
}