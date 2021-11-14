#include "detectPrimes.h"
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <thread>
#include <iostream>
#include <mutex>
#include <atomic>

std::mutex m;
class AtomicIndex {
    std::atomic<int> index = {-1};
public:
    int get() { 
        return index++; 
    }
};

// returns true if n is prime, otherwise returns false
// -----------------------------------------------------------------------------
// to get full credit for this assignment, you will need to adjust or even
// re-write the code in this function to make it multithreaded.
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

static void threaded_prime_calc(const std::vector<int64_t>& nums,std::vector<int64_t>& results, AtomicIndex &index) {
    for (long unsigned int local_count = index.get(); local_count < nums.size(); local_count = index.get()) {

#ifdef DEBUG
        std::cout << local_count << std::endl;
#endif
        if (is_prime(nums[local_count])) {
            std::unique_lock<std::mutex> lk(m);
            results.push_back(nums[local_count]);
        };
    }
    return;
}
//A much better, yet still simple solution, would be to parallelize the outer loop, but instead of
//giving each thread a fixed portion of the input to test, it would decide dynamically how many
//numbers each thread would process.For example, each thread could process the next number in
//the list, and if it is a prime, it would add it to the result vector.This would repeat until all
//numbers have been tested.
std::vector<int64_t>
detect_primes(const std::vector<int64_t> & nums, int n_threads)
{
  std::vector<int64_t> result;
  std::thread threads[n_threads];

  AtomicIndex index;
  
  for (int i = 0; i < n_threads; i++)
  {
      threads[i] = std::thread(threaded_prime_calc, std::ref(nums), std::ref(result), std::ref(index));
  }

  for (auto& t : threads)
  {
      t.join();
  }

  return result;
}
