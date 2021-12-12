// -------------------------------------------------------------------------------------
// this is the only file you need to edit
// -------------------------------------------------------------------------------------
//
// (c) 2021, Pavol Federl, pfederl@ucalgary.ca
// Do not distribute this file.

#include "fatsim.h"
#include <algorithm>
#include <cstdio>
#include <iostream>
#include <stack>

struct Entry {
  long block_value;
  long depth;

  Entry(long block_value, long depth)
  {
    this->block_value = block_value;
    this->depth = depth;
  }
};

void create_adjacency_list(
    const std::vector<long> & fat,
    std::vector<std::vector<long>> & adj_list,
    std::vector<long> & end_nodes)
{
  adj_list.resize(fat.size());
  for (int i = 0; i < fat.size(); i++) {
    if (fat[i] != -1) {
      adj_list[fat[i]].push_back(i);
    } else {
      end_nodes.push_back(i);
    }
  }
}

long dfs(long node, const std::vector<std::vector<long>> & adj_list)
{
  auto curr_stack = std::stack<Entry>();
  long longest_chain = 1;
  curr_stack.push(Entry(node, 1));

  while (! curr_stack.empty()) {
    auto curr_entry = curr_stack.top();
    curr_stack.pop();
    if (curr_entry.depth > longest_chain) { longest_chain = curr_entry.depth; }

    for (long i = 0; i < (long)adj_list[curr_entry.block_value].size(); i++) {
      auto new_entry = Entry(adj_list[curr_entry.block_value][i], curr_entry.depth + 1);
      curr_stack.push(new_entry);
    }
  }

  return longest_chain;
}

std::vector<long> fat_check(const std::vector<long> & fat)
{
  auto adj_list = std::vector<std::vector<long>>();
  auto end_nodes = std::vector<long>();
  auto results = std::vector<long>();

  create_adjacency_list(fat, adj_list, end_nodes);

  for (long node : end_nodes) { results.push_back(dfs(node, adj_list)); }

  std::sort(results.begin(), results.end());
  return results;
}
