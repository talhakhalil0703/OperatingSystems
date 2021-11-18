#include "detectPrimes.h"
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <atomic>
#include <thread>
#include <iostream>
#include <mutex>
#include <condition_variable>
static std::atomic<bool> finished(false);
static std::atomic<bool> current_is_prime(false);
static std::atomic<bool> got_answer(false);

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

void threaded_prime_calc(
    std::vector<int64_t>& results,
    uint64_t& index,
    simple_barrier& barrier,
    int number_of_threads,
    int thread_number,
    int64_t max,
    int64_t value_to_check_for_prime)
{
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

std::vector<int64_t>
detect_primes(const std::vector<int64_t>& nums, int n_threads)
{
    std::vector<int64_t> result;
    std::thread threads[n_threads];
    simple_barrier barrier(n_threads);
    uint64_t index = 0;
    int64_t max = 0;
    int64_t value_to_check_for_prime = 0;
    int64_t THRESHOLD = 100000;
    bool run_multi = false;

    if (n_threads > 1) {
        run_multi = true;
    }

   while(true){
       //If the number is big we'll use threads

        if (index == nums.size()) {
            // Reached the end
            break;
        }
        else {
            value_to_check_for_prime = nums[index++];
            
            //If simple answer compute here
            if ((value_to_check_for_prime < THRESHOLD) || !run_multi) {
                if (is_prime(value_to_check_for_prime)) {
                    result.push_back(value_to_check_for_prime);
                }
               continue;
            }
            else if (value_to_check_for_prime % 2 == 0) continue; // handle multiples of 2
            else if (value_to_check_for_prime % 3 == 0) continue; // handle multiples of 3
        }
        max = sqrt(value_to_check_for_prime);
        current_is_prime = true;

        for (int i = 0; i < n_threads; i++)
        {
            threads[i] = std::thread(threaded_prime_calc,
                std::ref(result),
                std::ref(index),
                std::ref(barrier),
                n_threads,
                i,
                max,
                value_to_check_for_prime);
        }

        for (auto& t : threads)
        {
            t.join();
        }
    }
    return result;
}