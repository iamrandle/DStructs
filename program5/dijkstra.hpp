
#ifndef DIJKSTRA_HPP_
#define DIJKSTRA_HPP_

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <limits>                    //Biggest int: std::numeric_limits<int>::max()
#include "array_queue.hpp"
#include "array_stack.hpp"
#include "heap_priority_queue.hpp"
#include "hash_graph.hpp"


namespace ics {


class Info {
  public:
    Info() { }

    Info(std::string a_node) : node(a_node) { }

    bool operator==(const Info &rhs) const { return cost == rhs.cost && from == rhs.from; }

    bool operator!=(const Info &rhs) const { return !(*this == rhs); }

    friend std::ostream &operator<<(std::ostream &outs, const Info &i) {
      outs << "Info[" << i.node << "," << i.cost << "," << i.from << "]";
      return outs;
    }

    //Public instance variable definitions
    std::string node = "?";
    int cost = std::numeric_limits<int>::max();
    std::string from = "?";
  };


  bool gt_info(const Info &a, const Info &b) { return a.cost < b.cost; }

  typedef ics::HashGraph<int> DistGraph;
  typedef ics::HeapPriorityQueue<Info, gt_info> CostPQ;
  typedef ics::HashMap<std::string, Info, DistGraph::hash_str> CostMap;
  typedef ics::pair<std::string, Info> CostMapEntry;


//Return the final_map as specified in the lecture-node description of
//  extended Dijkstra algorithm
  CostMap extended_dijkstra(const DistGraph &g, std::string start_node) {
        CostMap info_map;
        CostMap answer_map;
        for(auto val : g.all_nodes())
            info_map.put(val.first, Info(val.first));
        Info snode;
        snode.node = start_node;
        snode.cost = 0;
        info_map.put(snode.node, snode);
        CostPQ info_pq;
        for(auto val : info_map)
            info_pq.enqueue(val.second);
        while(info_map.size())
        {
            auto val = info_pq.dequeue();
            if(val.cost == std::numeric_limits<int>::max())
                break;
            if(answer_map.has_key(val.node))
                continue;
            answer_map.put(val.node,info_map.erase(val.node));
            for(auto d : g.out_nodes(val.node))
            {
                Info dnfo = info_map[d];
                if(dnfo.cost > g.edge_value(val.node, d) + val.cost)
                {
                    Info temp (d);
                    temp.cost = g.edge_value(val.node, d) + val.cost;
                    temp.from = val.node;
                    info_map.put(d,temp);
                    info_pq.enqueue(temp);
                }
            }
        }
        return answer_map;

  }


//Return a queue whose front is the start node (implicit in answer_map) and whose
//  rear is the end node
  ArrayQueue <std::string> recover_path(const CostMap &answer_map, std::string end_node) {
        ArrayStack<std::string> answerstack;
        ArrayQueue<std::string> path;
        answerstack.push(end_node);
        std::string next = answer_map[end_node].from;
        while (next != "?")
        {
            answerstack.push(next);
            next = answer_map[next].from;

        }
        while(answerstack.size())
            path.enqueue(answerstack.pop());
        return path;

  }


}

#endif /* DIJKSTRA_HPP_ */
