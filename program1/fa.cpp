
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


typedef ics::ArrayQueue<std::string>                InputsQueue;
typedef ics::ArrayMap<std::string,std::string>      InputStateMap;

typedef ics::ArrayMap<std::string,InputStateMap>    FA;
typedef ics::pair<std::string,InputStateMap>        FAEntry;

bool gt_FAEntry (const FAEntry& a, const FAEntry& b)
{return a.first<b.first;}

typedef ics::ArrayPriorityQueue<FAEntry,gt_FAEntry> FAPQ;

typedef ics::pair<std::string,std::string>          Transition;
typedef ics::ArrayQueue<Transition>                 TransitionQueue;


//Read an open file describing the finite automaton (each line starts with
//  a state name followed by pairs of transitions from that state: (input
//  followed by new state, all separated by semicolons), and return a Map
//  whose keys are states and whose associated values are another Map with
//  each input in that state (keys) and the resulting state it leads to.
const FA read_fa(std::ifstream &file) {
    FA fa;
    std::string line;
    while (getline(file,line))
    {
        std::vector<std::string> words = ics::split(line, ";");
        for(int i = 1; i < words.size()-1; i+=2)
            fa[words[0]].put(words[i],words[i+1]);

    }
    file.close();
    return fa;
}


//Print a label and all the entries in the finite automaton Map, in
//  alphabetical order of the states: each line has a state, the text
//  "transitions:" and the Map of its transitions.
void print_fa(const FA& fa) {
    std::cout<<"The Finite Automaton's Description"<<std::endl;
    FAPQ faPQ (gt_FAEntry);
        for(auto kv : fa)
            faPQ.enqueue(kv);
    for(auto i = faPQ.begin(); i != faPQ.end();i++)
        std::cout<<'\t'<<i->first<<" transitions: "<<i->second<<std::endl;
}
//Return a queue of the calculated transition pairs, based on the finite
//  automaton, initial state, and queue of inputs; each pair in the returned
//  queue is of the form: input, new state.
//The first pair contains "" as the input and the initial state.
//If any input i is illegal (does not lead to a state in the finite
//  automaton), then the last pair in the returned queue is i,"None".
TransitionQueue process(const FA& fa, std::string state, const InputsQueue& inputs) {
    TransitionQueue trans;
    for(auto val : inputs )
    {
        if (state == val)
        {
            Transition t("", state);
            trans.enqueue(t);
        }
        else if(!fa[state].has_key(val))
        {
            Transition t (val,"None");
            trans.enqueue(t);
            break;
        }
        else
        {
            state = fa[state][val];
            Transition t(val,state);
            trans.enqueue(t);
        }
    }
    return trans;
}
//Print a TransitionQueue (the result of calling the process function above)
// in a nice form.
//Print the Start state on the first line; then print each input and the
//  resulting new state (or "illegal input: terminated", if the state is
//  "None") indented on subsequent lines; on the last line, print the Stop
//  state (which may be "None").
void interpret(TransitionQueue& tq) {  //or TransitionQueue or TransitionQueue&&
    Transition t;
    std::cout<<"Starting a new simulation with description: ";
    for(auto val : tq)
    {
        if(val.first =="")
            std::cout<<val.second;
        else
            std::cout<< ";"<<val.first;
    }
    while(tq.size() > 0)
    {
        if (tq.peek().first == "")
            std::cout<<std::endl << "Start State = " << tq.peek().second << std::endl;
        else if (tq.peek().second == "None")
            std::cout << "\tInput = " << tq.peek().first << "; Illegal input: terminated" << std::endl;
        else
            std::cout << "\tInput = " << tq.peek().first << "; new state = " << tq.peek().second << std::endl;
        t = tq.dequeue();
    }
    std::cout<<"Stop State = "<< t.second<<std::endl<<std::endl;

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
        ics::safe_open(text_file,"Enter a finite automaton's file", "faparity.txt");
        FA fa = read_fa(text_file);
        print_fa(fa);

        ics::safe_open(text_file,"Enter a start stae input file", "fainputparity.txt");
        std::string line;
        while(getline(text_file,line))
        {
            InputsQueue inputs;
            std::vector<std::string> ins = ics::split(line,";");
            for(auto val : ins)
                inputs.enqueue(val);
            TransitionQueue tq = process(fa,inputs.peek(),inputs);
            interpret(tq);
        }
    }
    catch (ics::IcsError& e) {
        std::cout << e.what() << std::endl;
    }
    return 0;
}
