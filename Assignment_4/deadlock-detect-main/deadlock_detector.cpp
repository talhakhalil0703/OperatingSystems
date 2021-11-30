#include "deadlock_detector.h"
#include "common.h"
#include <iostream>

#if 1
#define DEBUG(x)                                                               \
    do {                                                                       \
        std::cerr << #x << ": " << x << std::endl;                             \
    } while (0)
#else
#define DEBUG(x)
#endif

std::vector<std::string> names;

class Graph {
public:
    std::vector<std::vector<int>> adj_list;
    std::vector<int> out_counts;
} graph;

void deadlock_detect(const Graph & gr, Result & res)
{
    std::vector<int> out;
    std::vector<int> zeros;

    for (int i = 0; i < gr.out_counts.size(); i++) {
        int count = gr.out_counts[i];
        out.push_back(count);
        if (count == 0) {
            zeros.push_back(i);
        }
    }

    while (zeros.size() > 0) {
        int n = zeros.back();
        zeros.pop_back();

        for (int n2 : gr.adj_list[n]) {
            out[n2] = out[n2] - 1;
            if (out[n2] == 0) {
                zeros.push_back(n2);
            }
        }
    }

    // Check if out count > 0; and then check if the node is actually a process
    // node, remove the p from the string and add it to the process list
    for (int i = 0; i < out.size(); i++) {
        if (out[i] > 0) {
            if (names[i][0] == 'p') {
                res.dl_procs.push_back(names[i].substr(1));
            }
        }
    }
}

void create_graph(const std::vector<std::string> & edges, Result & res)
{
    Word2Int converter = Word2Int();
    Graph gr = Graph();

    int amount_of_items = -1;
    bool incremented = false;
    int edge_iteration = 0;
    for (const std::string & edge : edges) {
        std::string m_edge = simplify(edge);
        std::vector<std::string> s = split(m_edge);

        std::string process_name = "p" + s.at(0);
        std::string resource_name = "r" + s.at(2);
        int int_process = converter.get(process_name);
        int int_resource = converter.get(resource_name);

        // Check to see if we have incremented, based on node names, if we have
        // then we should add onto the graph
        if (int_process > amount_of_items) {
            amount_of_items = int_process;
            incremented = true;
            names.push_back(process_name);
        }

        if (incremented) {
            gr.adj_list.push_back(std::vector<int>());
            gr.out_counts.push_back(0);
            incremented = false;
        }

        if (int_resource > amount_of_items) {
            amount_of_items = int_resource;
            incremented = true;
            names.push_back(resource_name);
        }

        if (incremented) {
            gr.adj_list.push_back(std::vector<int>());
            gr.out_counts.push_back(0);
            incremented = false;
        }

        if (s.at(1).compare("<-")) {
            // P -> R
            gr.adj_list[int_resource].push_back(int_process);
            gr.out_counts[int_process] = gr.out_counts[int_process] + 1;
        } else {
            // P <- R
            gr.adj_list[int_process].push_back(int_resource);
            gr.out_counts[int_resource] = gr.out_counts[int_resource] + 1;
        }

        deadlock_detect(gr, res);

        if (res.dl_procs.size() > 0) {
            res.edge_index = edge_iteration;
            return;
        }

        edge_iteration += 1;
    }
}

Result detect_deadlock(const std::vector<std::string> & edges)
{
    Result result;
    result.edge_index = -1;
    create_graph(edges, result);
    return result;
}
