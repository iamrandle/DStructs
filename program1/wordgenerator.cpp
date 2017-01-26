
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <limits>                           //I used std::numeric_limits<int>::max()
#include "ics46goody.hpp"
#include "array_queue.hpp"
#include "array_priority_queue.hpp"
#include "array_set.hpp"
#include "array_map.hpp"


typedef ics::ArrayQueue<std::string>         WordQueue;
typedef ics::ArraySet<std::string>           FollowSet;
typedef ics::pair<WordQueue,FollowSet>       CorpusEntry;
typedef ics::ArrayPriorityQueue<CorpusEntry> CorpusPQ;     //Convenient to supply gt at construction
typedef ics::ArrayMap<WordQueue,FollowSet>   Corpus;


//Return a random word in the words set (use in produce_text)
std::string random_in_set(const FollowSet& words) {
  int index = ics::rand_range(1, words.size());
  int i = 0;
  for (const std::string& s : words)
    if (++i == index)
      return s;
  return "?";
}

//Read an open file of lines of words (separated by spaces) and return a
//  Corpus (Map) of each sequence (Queue) of os (Order-Statistic) words
//  associated with the Set of all words that follow them somewhere in the
//  file.
Corpus read_corpus(int os, std::ifstream &file) {
    Corpus corpus;
    std::string line;
    std::vector<std::string> previousLine;
    while(getline(file,line))
    {
        std::vector<std::string> words;
        if(previousLine.size()>0)
        {
            for (int i = os; i > 0; i--)
                words.push_back(previousLine[previousLine.size() - i]);
            std::vector<std::string> words2 = ics::split(line, " ");
            words.insert(words.end(), words2.begin(), words2.end());
        }
        else
            words = ics::split(line, " ");
        for(int i = 0; i != words.size() - os;i++)
        {
            WordQueue wq;
            for(int j = 0; j < os; j++)
                wq.enqueue(words[i+j]);
            if(i + os < words.size())
                corpus[wq].insert(words[i+os]);
        }
        previousLine = words;
    }
    file.close();
    return corpus;
}

//Print "Corpus" and all entries in the Corpus, in lexical alphabetical order
//  (with the minimum and maximum set sizes at the end).
//Use a "can be followed by any of" to separate the key word from the Set of words
//  that can follow it.

//One queue is lexically greater than another, if its first value is smaller; or if
//  its first value is the same and its second value is smaller; or if its first
//  and second values are the same and its third value is smaller...
//If any of its values is greater than the corresponding value in the other queue,
//  the first queue is not greater.
//Note that the queues sizes are the same: each stores Order-Statistic words
//Important: Use iterators for examining the queue values: DO NOT CALL DEQUEUE.

bool queue_gt(const CorpusEntry& a, const CorpusEntry& b) {
    WordQueue c = a.first;
    WordQueue d = b.first;
    while (c.peek() == d.peek())
    {
        c.dequeue();
        d.dequeue();
    }
    return c.peek() < d.peek();
}

void print_corpus(const Corpus& corpus) {
    std::cout<<"Corpus has "<< corpus.size() <<" entries"<<std::endl;
    CorpusPQ corpusPQ(queue_gt);
    int min, max = 1;
    for(auto kv : corpus)
        corpusPQ.enqueue(kv);
    for(auto i = corpusPQ.begin(); i != corpusPQ.end();i++)
    {
        std::cout<<'\t'<<i->first<< "->"<<i->second<<std::endl;
        if(i->second.size() > max)
            max = i->second.size();
        if(i->second.size() < min)
            min = i->second.size();
    }
    std::cout<<"Corpus has "<< corpus.size() <<" entries"<<std::endl<<"min/max = "<<min<<"/"<<max<<std::endl;
}

//Return a Queue of words, starting with those in start and including count more
//  randomly selected words using corpus to decide which word comes next.
//If there is no word that follows the previous ones, put "None" into the queue
//  and return immediately this list (whose size is <= start.size() + count).
WordQueue produce_text(const Corpus& corpus, const WordQueue& start, int count) {
    WordQueue product = start;
    WordQueue qNext = start;
    for (int i = 0; i < count; i++)
    {
        if(!corpus.has_key(qNext))
        {
            product.enqueue("None");
            break;
        }
        std::string nextInProduct = random_in_set(corpus[qNext]);
        qNext.dequeue();
        qNext.enqueue(nextInProduct);
        product.enqueue(nextInProduct);
    }
    return product;
}

//Prompt the user for (a) the order statistic and (b) the file storing the text.
//Read the text as a Corpus and print it appropriately.
//Prompt the user for order statistic words from the text.
//Prompt the user for number of random words to generate
//Call the above functions to solve the problem, and print the appropriate information
int main() {
  try {
      int os = ics::prompt_int("Enter an order statistic",2);
      std::ifstream text_file;
      ics::safe_open(text_file,"Enter a file to process", "wginput1.txt");
      Corpus corpus = read_corpus(os,text_file);
      print_corpus(corpus);
      WordQueue wq;
      std::cout<<"Enter "<< os << " words to start with"<<std::endl;
      for(int i = 0; i < os;i++)
      {
          std::cout<<"Enter word "<<i+1<<" : ";
          std::string answer;
          std::cin>>answer;
          wq.enqueue(answer);
      }
      int count;
      std::cout<<"Enter number of words to generate: ";
      std::cin>>count;
      std::cout<<std::endl<<produce_text(corpus,wq,count)<<std::endl;
  } catch (ics::IcsError& e) {
    std::cout << e.what() << std::endl;
  }
  return 0;
}
