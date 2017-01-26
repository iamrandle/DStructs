
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include "ics46goody.hpp"
#include "array_queue.hpp"
#include "array_priority_queue.hpp"
#include "array_set.hpp"
#include "array_map.hpp"


typedef ics::ArraySet<std::string>                     States;
typedef ics::ArrayQueue<std::string>                   InputsQueue;
typedef ics::ArrayMap<std::string,States>              InputStatesMap;

typedef ics::ArrayMap<std::string,InputStatesMap>       NDFA;
typedef ics::pair<std::string,InputStatesMap>           NDFAEntry;

bool gt_NDFAEntry (const NDFAEntry& a, const NDFAEntry& b)
{return a.first<b.first;}

typedef ics::ArrayPriorityQueue<NDFAEntry,gt_NDFAEntry> NDFAPQ;

typedef ics::pair<std::string,States>                   Transitions;
typedef ics::ArrayQueue<Transitions>                    TransitionsQueue;


//Read an open file describing the non-deterministic finite automaton (each
//  line starts with a state name followed by pairs of transitions from that
//  state: (input followed by a new state, all separated by semicolons), and
//  return a Map whose keys are states and whose associated values are another
//  Map with each input in that state (keys) and the resulting set of states it
//  can lead to.
const NDFA read_ndfa(std::ifstream &file) {
    NDFA ndfa;
    std::string line;
    while (getline(file,line))
    {
        States states;
        std::vector<std::string> words = ics::split(line, ";");
        for(int i = 1; i < words.size()-1; i+=2)
        {
            states = ndfa[words[0]][(words[i])];
            states.insert(words[i+1]);
            ndfa[words[0]].put(words[i],states);
        }
        if(words.size() == 1)
            ndfa[words[0]];

    }
    file.close();

  return ndfa;
}

//Print a label and all the entries in the finite automaton Map, in
//  alphabetical order of the states: each line has a state, the text
//  "transitions:" and the Map of its transitions.
void print_ndfa(const NDFA& ndfa) {
    std::cout<<"The Finite Automaton's Description"<<std::endl;
    NDFAPQ ndfaPQ (gt_NDFAEntry);
    for(auto kv : ndfa)
        ndfaPQ.enqueue(kv);
    for(auto i = ndfaPQ.begin(); i != ndfaPQ.end();i++)
        std::cout<<'\t'<<i->first<<" transitions: "<<i->second<<std::endl;
}

//Return a queue of the calculated transition pairs, based on the non-deterministic
//  finite automaton, initial state, and queue of inputs; each pair in the returned
//  queue is of the form: input, set of new states.
//The first pair contains "" as the input and the initial state.
//If any input i is illegal (does not lead to any state in the non-deterministic finite
//  automaton), ignore it.
TransitionsQueue process(const NDFA& ndfa, std::string state, const InputsQueue& inputs) {
    TransitionsQueue trans;
    States s;
    for(auto val : inputs )
    {
        if (state == val)
        {
            s.insert(val);
            Transitions t("", s);
            trans.enqueue(t);
        }
        else if (s.contains("None"))
            break;
        else
        {
            States tempStates;
            for(auto val2 : s)
            {
                if(ndfa[val2].has_key(val))
                    for(auto pt = ndfa[val2][val].begin(); pt != ndfa[val2][val].end(); pt++)
                        tempStates.insert(*pt);
            }
            if(!tempStates.size())
                tempStates.insert("None");
            s = tempStates;
            Transitions t(val,s);
            trans.enqueue(t);
        }
    }
    return trans;
}

//Print a TransitionsQueue (the result of calling process) in a nice form.
//Print the Start state on the first line; then print each input and the
//  resulting new states indented on subsequent lines; on the last line, print
//  the Stop state.
void interpret(TransitionsQueue& tq) {  //or TransitionsQueue or TransitionsQueue&&
    Transitions t;
    std::cout<<"Starting a new simulation with description: ";
    for(auto val : tq)
    {
        if(val.first =="")
            for(auto val2 : val.second)
                std::cout<<val2;
        else
            std::cout<< ";"<<val.first;
    }
    while(tq.size() > 0 )
    {
        if (tq.peek().first == "")
            std::cout<<std::endl << "Start State = " << tq.peek().second << std::endl;
        else if (tq.peek().second.contains("None"))
            std::cout << "\tInput = " << tq.peek().first << "; Illegal input: terminated" << std::endl;
        else
            std::cout << "\tInput = " << tq.peek().first << "; new state = " << tq.peek().second << std::endl;
        t = tq.dequeue();
    }
    std::cout<<"Stop State(s) = "<< t.second<<std::endl<<std::endl;
}

//Prompt the user for a file, create a finite automaton Map, and print it.
//Prompt the user for a file containing any number of simulation descriptions
//  for the finite automaton to process, one description per line; each
//  description contains a start state followed by its inputs, all separated by
//  semicolons.
//Repeatedly read a description, print that description, put each input in a
//  Queue, process the Queue and print the results in a nice form.
int main() {
  try {
      std::ifstream text_file;
      ics::safe_open(text_file,"Enter a non-deterministic finite automaton's file", "ndfaendin01.txt");
      NDFA ndfa = read_ndfa(text_file);
      print_ndfa(ndfa);

      ics::safe_open(text_file,"Enter a start stae input file", "ndfainputendin01.txt");
      std::string line;
      while(getline(text_file,line))
      {
          InputsQueue inputs;
          std::vector<std::string> ins = ics::split(line,";");
          for(auto val : ins)
              inputs.enqueue(val);
          TransitionsQueue tq = process(ndfa,inputs.peek(),inputs);
          interpret(tq);
      }
  } catch (ics::IcsError& e) {
    std::cout << e.what() << std::endl;
  }
  return 0;
}
