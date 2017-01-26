
#include <string>
#include <iostream>
#include <fstream>
#include "ics46goody.hpp"
#include "array_queue.hpp"
#include "hash_graph.hpp"
#include "dijkstra.hpp"


std::string get_node_in_graph(const ics::DistGraph& g, std::string prompt, bool allow_QUIT) {
  std::string node;
  for(;;) {
    node = ics::prompt_string(prompt + " (must be in graph" + (allow_QUIT ? " or QUIT" : "") + ")");
    if ((allow_QUIT && node == "QUIT") || g.has_node(node))
      break;
  }
  return node;
}

int main() {
  try {
      std::ifstream in_file;
      ics::safe_open(in_file,"Enter a graph file's name", "flightcost.txt");
      ics::HashGraph<int> g;
      g.load(in_file,";");
      in_file.close();
      std::cout<<g<<std::endl;

      auto answer = ics::extended_dijkstra(g,get_node_in_graph(g,"Enter Start Node ", false));
      std::cout<<answer<<std::endl;
      std::string node_or_quit;
      for(;;)
      {
          node_or_quit = get_node_in_graph(g,"Enter Stop Node ", true);
          if(node_or_quit == "QUIT")
              break;
          auto path = ics::recover_path(answer, node_or_quit);
          std::cout << path <<std::endl;
      }

  } catch (ics::IcsError& e) {
    std::cout << e.what() << std::endl;
  }

  return 0;
}
