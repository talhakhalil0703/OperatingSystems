// this is the ONLY file you should edit and submit to D2L

#include "deadlock_detector.h"
#include "common.h"
#include <iostream>

class Graph {
public:
    std::unordered_map<std::string, std::vector<std::string>> adj_list;
    std::unordered_map<std::string, int> out_counts;
} graph;

void print_out_vector(const std::unordered_map<std::string, int> & out)
{
    for (auto & it : out) {
        std::cout << it.first << ":" << it.second << " ";
    }

    std::cout << std::endl;
}

void print_vector(const std::vector<std::string> & vec)
{
    for (std::string item : vec) {
        std::cout << item;
    }
    std::cout << std::endl;
}

void is_deadlock(const Graph & gr, std::vector<std::string> & dl_procs)
{
    // out = out_counts deep copy so we can modify it
    std::unordered_map<std::string, int> out;
    std::vector<std::string> zeros;

    // Deep copy:
    for (auto & it : gr.out_counts) {
        out[it.first] = it.second;
    }

    // Find all zeros:
    for (auto & it : gr.adj_list) {
        if (gr.out_counts.find(it.first) == gr.out_counts.end()) {
            zeros.push_back(it.first);
        }
    }

    // std::cout << "Printing Zeros: " << zeros.size() << " ";
    // print_vector(zeros);
    // std::cout << "Printing Copied Out: " << out.size() << " ";
    // print_out_vector(out);

    while (zeros.size() > 0) {
        std::string n = zeros.back();
        zeros.pop_back();
        auto vec = gr.adj_list;
        for (std::string item : vec[n]) {
            out[item] = out[item] - 1;
            if (out[item] == 0) {
                zeros.push_back(item);
            }
        }
    }
    // zeros[] find all nodes in graph with outdegree == 0

    for (auto & it : out) {
        if (it.second > 0 && it.first[0] == 'p') {
            dl_procs.push_back(it.first);
        }
    }

    // for (std::string proc : dl_procs) {
    //     std::cout << proc << " ";
    // }
    // std::cout << std::endl;
}

void create_graph(
    const std::vector<std::string> & edges, Graph & gr, Result & result)
{

    int i = 0;
    for (const std::string & edge : edges) {
        std::string m_edge = simplify(edge);
        std::vector<std::string> s = split(m_edge);
        std::string resource_name = "r" + s.at(2);
        std::string process_name = "p" + s.at(0);

        // std::cout << edge << std::endl;
        if (s.at(1).compare("<-")) {
            // P -> R
            if (gr.adj_list[resource_name].size() == 0) {
                gr.adj_list[resource_name] = std::vector<std::string>();
            }
            gr.adj_list[resource_name].push_back(process_name);

            gr.out_counts[process_name] = gr.out_counts[process_name] + 1;
        } else {
            // P <- R
            if (gr.adj_list[process_name].size() == 0) {
                gr.adj_list[process_name] = std::vector<std::string>();
            }
            gr.adj_list[process_name].push_back(resource_name);
            gr.out_counts[resource_name] = gr.out_counts[resource_name] + 1;
        }

        is_deadlock(gr, result.dl_procs);
        // std::cout << "dl_procx size " << result.dl_procs.size()
        //           << std::endl;
        if (result.dl_procs.size() > 0) {
            result.edge_index = i;
            return;
        }

        i++;
        if (i % 100 == 0) {
            std::cout << i << std::endl;
        }
    }
}

void print_graph(const Graph & gr)
{
    std::cout << "Printing Graph" << std::endl;

    for (auto & it : gr.adj_list) {
        std::cout << it.first << ":";
        for (std::string v : it.second) {
            std::cout << v << " ";
        }
        std::cout << std::endl;
    }

    std::cout << "Out Counts" << std::endl;

    for (auto & it : gr.out_counts) {
        std::cout << it.first << ":" << it.second << std::endl;
    }
}

Result detect_deadlock(const std::vector<std::string> & edges)
{

    Graph gr;
    Result result;
    result.edge_index = -1;
    create_graph(edges, gr, result);
    return result;
}
