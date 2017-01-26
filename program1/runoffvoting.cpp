
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <limits>                    //Biggest int: std::numeric_limits<int>::max()
#include "ics46goody.hpp"
#include "array_queue.hpp"
#include "array_priority_queue.hpp"
#include "array_set.hpp"
#include "array_map.hpp"


typedef ics::ArrayQueue<std::string>              CandidateQueue;
typedef ics::ArraySet<std::string>                CandidateSet;
typedef ics::ArrayMap<std::string,int>            CandidateTally;

typedef ics::ArrayMap<std::string,CandidateQueue> Preferences;
typedef ics::pair<std::string,CandidateQueue>     PreferencesEntry;
typedef ics::ArrayPriorityQueue<PreferencesEntry> PreferencesEntryPQ; //Must supply gt at construction

typedef ics::pair<std::string,int>                TallyEntry;
typedef ics::ArrayPriorityQueue<TallyEntry>       TallyEntryPQ;



//Read an open file stating voter preferences (each line is (a) a voter
//  followed by (b) all the candidates the voter would vote for, in
//  preference order (from most to least preferred candidate, separated
//  by semicolons), and return a Map of preferences: a Map whose keys are
//  voter names and whose values are a queue of candidate preferences.
Preferences read_voter_preferences(std::ifstream &file) {

    Preferences vPref;
    std::string line;
    while(getline(file,line))
    {
      std::vector<std::string> words = ics::split(line, ";");
      for(int i = 1; i < words.size();i++)
        vPref[words[0]].enqueue(words[i]);
    }
    file.close();
    return vPref;
}


//Print a label and all the entries in the preferences Map, in alphabetical
//  order according to the voter.
//Use a "->" to separate the voter name from the Queue of candidates.
void print_voter_preferences(const Preferences& preferences) {
    std::cout<<"Voter: source -> queue[Preferences]"<<std::endl;
    PreferencesEntryPQ preferencesEntryPQ (preferences,[]( const PreferencesEntry& a, const PreferencesEntry& b){return a.first<b.first;});
    for(auto i = preferencesEntryPQ.begin(); i != preferencesEntryPQ.end();i++)
        std::cout<<'\t'<<i->first<< "->"<<i->second<<std::endl;
}


//Print the message followed by all the entries in the CandidateTally, in
//  the order specified by has_higher_priority: i is printed before j, if
//  has_higher_priority(i,j) returns true: sometimes alphabetically by candidate,
//  other times by decreasing votes for the candidate.
//Use a "->" to separate the candidat name from the number of votes they
//  received.
void print_tally(std::string message, const CandidateTally& tally, bool (*has_higher_priority)(const TallyEntry& i,const TallyEntry& j)) {
    std::cout << "\n" << message  <<std::endl;
    for (auto kv : TallyEntryPQ(tally,has_higher_priority))
        std::cout << "  " << kv.first << " --> " << kv.second << std::endl;
}


//Return the CandidateTally: a Map of candidates (as keys) and the number of
//  votes they received, based on the unchanging Preferences (read from the
//  file) and the candidates who are currently still in the election (which changes).
//Every possible candidate should appear as a key in the resulting tally.
//Each voter should tally one vote: for their highest-ranked candidate who is
//  still in the the election.
CandidateTally evaluate_ballot(const Preferences& preferences, const CandidateSet& candidates) {
    CandidateTally can;
    for (auto kv : preferences) {
        while (!candidates.contains(kv.second.peek()))
            kv.second.dequeue();
        can[kv.second.peek()]+=1;
    }
    return can;
}


//Return the Set of candidates who are still in the election, based on the
//  tally of votes: compute the minimum number of votes and return a Set of
//  all candidates receiving more than that minimum; if all candidates
//  receive the same number of votes (that would be the minimum), the empty
//  Set is returned.
CandidateSet remaining_candidates(const CandidateTally& tally) {
    CandidateSet candidateSet;
    TallyEntry tallyEntry;
    for(auto i = tally.begin(); i != tally.end();i++)
    {
        if (i == tally.begin() || i->second < tallyEntry.second)
            tallyEntry = *i;
        candidateSet.insert(i->first);
    }
    candidateSet.erase(tallyEntry.first);
    return candidateSet;
}


//Prompt the user for a file, create a voter preference Map, and print it.
//Determine the Set of all the candidates in the election, from this Map.
//Repeatedly evaluate the ballot based on the candidates (still) in the
//  election, printing the vote count (tally) two ways: with the candidates
//  (a) shown alphabetically increasing and (b) shown with the vote count
//  decreasing (candidates with equal vote counts are shown alphabetically
//  increasing); from this tally, compute which candidates remain in the
//  election: all candidates receiving more than the minimum number of votes;
//  continue this process until there are less than 2 candidates.
//Print the final result: there may 1 candidate left (the winner) or 0 left
//   (no winner).
int main() {
  try {
    std::ifstream text_file;
    ics::safe_open(text_file,"Enter a voter preferences file's name", "votepref1.txt");
    Preferences vPref = read_voter_preferences(text_file);
    print_voter_preferences(vPref);
     bool keepGoing = true;
      CandidateTally can;
      for (auto kv : vPref) {
          can[kv.second.peek()] += 1;
      }
      int k = 1;
      while(keepGoing)
      {
          print_tally("Vote count on ballot #"+ std::to_string(k)+" with candidates (alphabetically ordered)", can,
                      [](const TallyEntry &i, const TallyEntry &j) {
                          return (i.first == j.first ? i.second < j.second : i.first < j.first);
                      });
          print_tally("Vote count on ballot #"+ std::to_string(k)+" with candidates (numerically ordered)", can,
                      [](const TallyEntry &i, const TallyEntry &j) {
                          return (i.second == j.second ? i.first < j.first : i.second > j.second);
                      });
          if(can.size() == 2 )
          {
              keepGoing = false;
              auto j = can.begin();
              TallyEntry person1 = *j;
              TallyEntry person2 = *++j;
              if(person1.second == person2.second)
                  std::cout<<"No winner: election is a tie among all candidates remaining on the last ballot"<<std::endl;
              else
                  person1.second > person2.second ? std::cout<< person1.first << " is the winner!"<<std::endl : std::cout<< person2.first << " is the winner!"<<std::endl;
          }
          else
              can = evaluate_ballot(vPref,remaining_candidates(can));
          k++;
      }
  } catch (ics::IcsError& e) {
    std::cout << e.what() << std::endl;
  }
  return 0;
}
