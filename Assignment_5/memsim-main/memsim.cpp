// -------------------------------------------------------------------------------------
// this is the only file you need to edit
// -------------------------------------------------------------------------------------
//
// (c) 2021, Pavol Federl, pfederl@ucalgary.ca
// Do not distribute this file.

#include "memsim.h"
#include <cassert>
#include <cmath>
#include <iostream>
#include <list>
#include <set>
#include <unordered_map>

struct Partition {
  int64_t tag;
  int64_t size, addr;

  Partition(int64_t size, int64_t addr)
  {
    this->size = size;
    this->addr = addr;
  }
};

typedef std::list<Partition>::iterator PartitionRef;

struct scmp {
  bool operator()(const PartitionRef & c1, const PartitionRef & c2) const
  {
    if (c1->size == c2->size)
      return c1->addr < c2->addr;
    else
      return c1->size > c2->size;
  }
};
struct Simulator {
  // List of all partitions in a linked list
  std::list<Partition> all_blocks;
  // Set of sorted partitions by size/address that are free
  std::set<PartitionRef, scmp> free_blocks;
  // Unordered map that allows for quick access to all tagged partitions
  std::unordered_map<int64_t, std::vector<PartitionRef>> tagged_blocks;
  int64_t size_of_pages;
  int64_t requested_pages = 0;

  // Constructor
  Simulator(int64_t page_size)
  {
    size_of_pages = page_size;
    Partition part = Partition(0, 0);
    part.tag = -1;
    all_blocks.insert(all_blocks.end(), part);
  }

  void allocate(int64_t tag, int64_t size)
  {
    PartitionRef partition_to_use = all_blocks.end();
    // Pseudocode for allocation request:
    // - search through the list of partitions from start to end, and
    //   find the largest partition that fits requested size
    //     - in case of ties, pick the first partition found
    if (free_blocks.begin() != free_blocks.end()) {
      if ((*(free_blocks.begin()))->size > size) {
        partition_to_use = *free_blocks.begin();
        allocate_split(partition_to_use, tag, size);
        return;
      }
    }

    // - if no suitable partition found:
    if (partition_to_use == all_blocks.end()) {
      //     - get minimum number of pages from OS, but consider the
      //       case when last partition is free
      auto last_it = std::prev(all_blocks.end());
      int64_t last_it_size = 0;
      if (last_it->tag == -1) {
        last_it_size += last_it->size;
      }

      int64_t pages_to_request = std::ceil(1.0 * (size - last_it_size) / size_of_pages);
      requested_pages += pages_to_request;

      //     - add the new memory at the end of partition list
      // If the last memory was indeed free
      if (last_it_size != 0) {
        free_blocks.erase(last_it);
        last_it->size += pages_to_request * size_of_pages;
        free_blocks.insert(last_it);
      } else {
        // If the last memory was not free
        int64_t new_addr = std::prev(all_blocks.end())->addr + std::prev(all_blocks.end())->size;
        Partition part = Partition(pages_to_request * size_of_pages, new_addr);
        part.tag = -1;
        all_blocks.insert(all_blocks.end(), part);
        free_blocks.insert(std::prev(all_blocks.end()));
      }
      //     - the last partition will be the best partition
      partition_to_use = *free_blocks.begin();
    }
    allocate_split(partition_to_use, tag, size);
    return;
  }

  void allocate_split(PartitionRef partition_to_use, int64_t tag, int64_t size)
  {
    // - split the best partition in two if necessary
    //     - mark the first partition occupied, and store the tag in it
    //     - mark the second partition free
    // If partion size is the same size as we exactly need it
    PartitionRef saved_ref = partition_to_use;
    if (partition_to_use->size == size) {
      free_blocks.erase(partition_to_use);
      partition_to_use->tag = tag;
    } else {
      // split into two
      // First half used by process
      int64_t original_free_partition_size = partition_to_use->size;
      // Second hald is the free memory
      int64_t remaining_free_size = original_free_partition_size - size;
      int64_t new_address = partition_to_use->addr + size;
      Partition free_part = Partition(remaining_free_size, new_address);
      free_part.tag = -1;
      free_blocks.erase(partition_to_use);
      partition_to_use->size = size;
      partition_to_use->tag = tag;

      // Add new partition onto all blocks as well as add it to free blocks set
      all_blocks.insert(std::next(partition_to_use), free_part);
      free_blocks.insert(std::next(partition_to_use));
    }

    // If not already in tagged_blocks then we need to create and add the new vector
    if (tagged_blocks.find(partition_to_use->tag) == tagged_blocks.end()) {
      auto vector_of_partitions = std::vector<PartitionRef>();
      vector_of_partitions.insert(vector_of_partitions.end(), partition_to_use);
      tagged_blocks.insert({ partition_to_use->tag, vector_of_partitions });
    } else {
      tagged_blocks[partition_to_use->tag].push_back(partition_to_use);
    }
  }

  void deallocate(int64_t tag)
  {
    // Pseudocode for deallocation request:
    // - for every partition
    //     - if partition is occupied and has a matching tag:
    //         - mark the partition free
    //         - merge any adjacent free partitions
    if (tagged_blocks.find(tag) == tagged_blocks.end()) {
      return;
    }

    auto vector_of_partions = tagged_blocks.find(tag)->second;

    for (auto & part : vector_of_partions) {
      // Mark the partition free
      part->tag = -1;
      // Check if prev is free
      auto previous = std::prev(part);
      auto next = std::next(part);
      bool prev_free = false;
      bool next_free = false;

      if (part == all_blocks.begin()) {
        prev_free = false;
      } else if (previous->tag == -1) {
        prev_free = true;
      }

      if (next == all_blocks.end()) {
        next_free = false;
      } else if (next->tag == -1){
        next_free = true;
      }
    

      if (! prev_free && ! next_free) {
        // Add onto free list
        part->tag = -1;
        free_blocks.insert(part);
      } else if (! prev_free && next_free) {
        int64_t next_size = next->size;
        free_blocks.erase(next);
        part->tag = -1;
        part->size += next_size;
        all_blocks.erase(next);
        free_blocks.insert(part);
      } else if (prev_free && ! next_free) {
        free_blocks.erase(previous);
        previous->size += part->size;
        all_blocks.erase(part);
        free_blocks.insert(previous);
      } else if (prev_free && next_free) {
        free_blocks.erase(previous);
        free_blocks.erase(next);
        previous->size += part->size + next->size;
        all_blocks.erase(next);
        all_blocks.erase(part);
        free_blocks.insert(previous);
      }
    }
    // Remove whole vector
    // Remove the whole map
    tagged_blocks.erase(tag);
  }

  MemSimResult getStats()
  {
    MemSimResult result;
    result.max_free_partition_size = 0;
    result.max_free_partition_address = 0;
    if (free_blocks.size() > 0) {
      auto partition = *free_blocks.begin();
      result.max_free_partition_size = partition->size;
      result.max_free_partition_address = partition->addr;
    }
    result.n_pages_requested = requested_pages;
    return result;
  }
};

MemSimResult mem_sim(int64_t page_size, const std::vector<Request> & requests)
{
  Simulator sim(page_size);
  for (const auto & req : requests) {
    if (req.tag < 0) {
      sim.deallocate(-req.tag);
    } else {
      sim.allocate(req.tag, req.size);
    }
  }
  return sim.getStats();
}