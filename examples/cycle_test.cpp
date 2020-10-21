// cycle_test.cpp
//
//


#include "bitstrm/reg.hpp"
#include "bitstrm/alloced_bref.hpp"
#include "bitstrm/utility.hpp"
#include "bitstrm/bit_int_itr.hpp"
#include "bitstrm/TimeFixture.hpp"
#include <boost/lexical_cast.hpp>
#include <iostream>
#include <string>
#include <utility>
#include <random>

using namespace bitstrm;
using namespace std;


unsigned bitstrm::TimeFixture::s_timer_number = 0;
mt19937                       generator(3);

 std::ostream&
 print_human_bits(std::ostream& out, unsigned long long bits){
    
    unsigned long long  bytes = bits/8;

    if(bytes < 2){
      out << bits << " bits";
    } else if(bytes < 1024){
      unsigned long long whole_bytes = bits/8;
      out << whole_bytes << " bytes";
      unsigned long long remain = (bits - (whole_bytes*8));
      if(remain )
        out << " " << remain << " bits";
    } else if(bytes < 1024*1024){
      out << double(bits) / (8*1024) << " kbytes";
    } else if(bytes < 1024*1024*1024){
      out << double(bits) / (8*1024*1024) << " mbytes";
    } else {
      out << double(bits)/(double(8)*1024*1024*1024) << " gbytes";
    }
    
    return out;
  }


const unsigned long long c_large_size = 1024*1024*1024*16ULL;

// stateless_graph_following_algorithm
//
// a simple stateless discoverer of cycles, it works by traversing a
// path a two speeds 1x and 2x until the 2x catches up to the 1x, then
// goes for another lap to determine the size of cycle it captured
//
// returns the size of the cycle
template<class ITR>
unsigned long long stateless_graph_following_algorithm(ITR beg, ITR cur){
  
  auto x1 = cur;
  auto x2 = beg + *cur;
  if(x1 != x2){
    x2 = beg + *x2;
    while(x1 != x2){
      x2 = beg + *x2;
      x2 = beg + *x2; // advance 2
      x1 = beg + *x1; // advance 1
    }
  }
  
  // found our loop single advance through loop
  unsigned long long count = 1;
  x2 = beg + *x1;
  while(x1 != x2){
    x2 = beg + *x2;
    ++count;
  }
  return count;
}

// graph_following_algorithm
//
// cycle detection occurs starting on beg_off  when traversed vector indicates that the next
// node will either complete a new_cycle or drop into a previously traversed cycle
//
// In the first case the function will return the count of the discovered cycle, updating
// traversed.  In the second case the function will return 0 and mark the path leading up
// to the previously discovered cycle in traversed

template<class ITR>
unsigned long long graph_following_algorithm(ITR beg, unsigned long long beg_off, vector<bool>& traversed){
  //
  if(traversed[beg_off])
    return 0;

  static alloced_bref buf(traversed.size());
  buf.zero();
  bit_int_itr<1, ureg> new_cycle(buf);
  
  auto off = beg_off;  
  auto cur = beg + off;
  
  unsigned long long path_count = 0;
  while(traversed[off] == false){
    new_cycle[off] = traversed[off] = true;
    ++path_count;
    off = *cur;
    if(traversed[off] && *(new_cycle + off) == 0)
      break;
    if(*(new_cycle + off) == 1){
      // we have encountered a cycle @ hit
      // how far did we go before cycle ?
      auto hit = beg + off;
      // reset cur
      cur = beg + beg_off; 
      unsigned long pre_cyclic_count = 0;
      while(cur != hit){
        ++pre_cyclic_count;
        cur = beg + *cur;
      }
      // a new cycle size
      return path_count - pre_cyclic_count;
    }
    cur = beg + off;
  }
  return 0; // this return ==> encountered existing graph
}

template<class ITR>
unsigned long long graph_following_algorithm_print(ITR beg, unsigned long long beg_off, vector<bool>& traversed){
  //
  cout << beg_off << ":";
  if(traversed[beg_off]){
    cout << unsigned(*(beg + beg_off)) << ",..."  << endl;
    return 0;
  }

  static alloced_bref buf(traversed.size());
  buf.zero();
  bit_int_itr<1, ureg> new_cycle(buf);
  
  auto off = beg_off;  
  auto cur = beg + off;
  
  unsigned long long path_count = 0;
  while(traversed[off] == false){
    new_cycle[off] = traversed[off] = true;
    ++path_count;
    off = *cur;
    if(traversed[off] && *(new_cycle + off) == 0)
      break;
    if(*(new_cycle + off) == 1){
      // we have encountered a cycle @ hit
      // how far did we go before cycle ?
      auto hit = beg + off;
      // reset cur
      cur = beg + beg_off; 
      unsigned long pre_cyclic_count = 0;
      while(cur != hit){
        cout << ',' << unsigned(*cur);
        ++pre_cyclic_count;
        cur = beg + *cur;
      }
      cout << ",^" << unsigned(*hit);
      cur = beg + *cur;
      while(cur != hit){
        cout << ',' << unsigned(*cur);
        cur = beg + *cur;
      }
      auto cycle_size = path_count - pre_cyclic_count; 
      cout << "[" << cycle_size << "]" << endl;
      // a new cycle size
      return cycle_size;
    }
    cur = beg + off;
  }
  cout << unsigned(*cur) << ",..." << endl;
  return 0; // this return ==> encountered existing graph
}

struct cycle {
  unsigned long long m_elements;  // nodes of cycle 
  unsigned long long m_count;     // encountered count of this cycle

  cycle() : m_count(1){}
  
  void update_smaller(unsigned long long smaller_candidate);
  void update_larger (unsigned long long larger_candidate);

  bool
  operator==(const cycle& rhs){
    return m_elements == rhs.m_elements
      && m_count == rhs.m_count;
  }

  bool
  operator!=(const cycle& rhs){ return !(operator==(rhs)); }
};

ostream& operator<<(ostream& lhs, const cycle& rhs) {
  lhs << "λ: " <<  rhs.m_elements << " @" << rhs.m_count;
  return lhs;
}

void
cycle::update_smaller(unsigned long long smaller_candidate){
  if(smaller_candidate == m_elements)
      ++m_count;
    else if(m_elements > smaller_candidate){
      m_elements = smaller_candidate;
      m_count = 1;
    }
}

void
cycle::update_larger(unsigned long long larger_candidate){
  if(larger_candidate == m_elements)
      ++m_count;
    else if(m_elements < larger_candidate){
      m_elements = larger_candidate;
      m_count = 1;
    }
}

template<class ITR>
pair<cycle, cycle> 
graph_min_max_cycle(ITR beg, ITR end){

  pair<cycle, cycle> min_max;
  
  min_max.first.m_count = min_max.second.m_count = 1;
  unsigned long long size = distance(beg, end);
  vector<bool> traversed(size, false);

  // min_max.first.m_elements = min_max.second.m_elements = graph_following_algorithm_print(beg, 0, traversed);
  min_max.first.m_elements = min_max.second.m_elements = graph_following_algorithm(beg, 0, traversed);
  for(unsigned long long i = 1; i < size; ++i ){
    // auto current_cycle = graph_following_algorithm_print(beg, i, traversed);
    auto current_cycle = graph_following_algorithm(beg, i, traversed);
    if(current_cycle != 0){
      min_max.first.update_smaller(current_cycle);
      min_max.second.update_larger(current_cycle);
    }
  }

  return min_max;
}


template<typename LINK>
void
generate_measure_and_check(std::vector<LINK>& graph, unsigned bsize){
  
  // begin with a graph lableled [0, n-1]
  // or each ith node just points to itself
  {
    string message(string("assign ") + boost::lexical_cast<string>(graph.size()) + " pointers");
    TimeFixture f(message.c_str());
    unsigned long long i = 0;
    for( auto& v : graph)
      v = i++;
  }

  // {n0->n0, n1->n1, ..., nx->nx}
  // so far we have a trivial graph each node pointing exactly to itself
  // we could create other trivial graphs, a simple cyclic ladder where
  // each node points to the next until the last which points to the 0th
  // but to get anything interesting we need to introduce some randomness

  //{n0->ni, ..., ni->nj, ..., nj->nk, ...}
  // shuffle it, or swap each connection with another
  {
    string message("shuffle pointers forming nontrivial graph");
    TimeFixture f(message.c_str());
    for(int i = 0 ; i < 8; ++i) {
      shuffle(graph.begin(), graph.end(), generator);
    }
  }

  // {n0->ni, ..., ni->nk, ..., nj->nk, ...}
  // reduce it, replace (hence likely orphan nodes) by replacing links
  // with a random assignment
  {
    unsigned long long replacements = min(1ULL, (unsigned long long)(0.5*graph.size()));
    string message("orphan nodes forming nontrivial graph");
    TimeFixture f(message.c_str());
    uniform_int_distribution<int> distr(0, graph.size() -1);
    for(; replacements; --replacements)
      graph[distr(generator)] = distr(generator);
  }

  // copy the vector into a suitable bitstrm
  alloced_bref buf(bsize * graph.size());
  {
    string message("copy to bitstrm");
    TimeFixture f(message.c_str());
    dbit_int_itr<ureg> cur(buf, bsize);
    copy(graph.begin(), graph.end(), cur);
  }
  
  pair<cycle, cycle> min_max;
  {
    string message("determine cycles on std vector");
    TimeFixture f(message.c_str());
    // cout << "vector:\n" << endl;
    min_max = graph_min_max_cycle(graph.begin(), graph.end());
  }

  pair<cycle, cycle> min_max_alt;
  {
    string message("determine cycles on bitstrm vector");
    TimeFixture f(message.c_str());
    dbit_int_citr<ureg> beg(buf, bsize);
    dbit_int_citr<ureg> end(beg + graph.size());
    // cout << "bitstrm:\n" << endl;
    min_max_alt = graph_min_max_cycle(beg, end);
  }
  
  if(min_max.first != min_max_alt.first
     || min_max.second != min_max_alt.second){
    cout << "Both methods are SUPPPOSED to return the same result!" << endl;
    cout << "std min: " << min_max.first << " max: " << min_max.second << endl;
    cout << "alt min: " << min_max_alt.first << " max: " << min_max_alt.second << endl;
  } else {
    cout << "std min: " << min_max.first << " max: " << min_max.second << endl;
  }  
}


int main(int argc , const char** argv){

  cout << "This simple algorithm to obtains the count of the very\n"
    "smallest cycle, and the very largest cycle of a psudo-random\n"
    "generated singely linked graph in a way that that requires only\n"
    "a couple bits per node of information for current traversal state.\n"
       << endl << endl;
     
  unsigned long long graph_size = 0;
  if(argc != 2  || ((graph_size = strtoll(argv[1], 0, 10)) > c_large_size)){
    cout << "USAGE: ./cycle_test SIZE" << endl;
    cout <<
      "Pick a reasonable (and non negative) SIZE : [1, "
         << c_large_size << "]\nexiting ..." << endl;
    return -1;
  }
  
  cout << "graph size: " << graph_size << endl;

  unsigned bsize = min_bits(graph_size-1);
  unsigned bytes = (min_bits(graph_size-1) + 7)/8;
  unsigned register_sizes[] = {1, 1, 2, 4, 4, 8, 8, 8, 8};
  bytes = register_sizes[bytes];
  if(bytes == 0){
    cout << "thats a trivial answer 0 nodes <==> zero length cycle" << endl;
    return 0;
  }
  
  cout << "fits in ";
  print_human_bits(cout, bsize*graph_size)   << " with bitstrm or in ";
  print_human_bits(cout, bytes*graph_size*8) << " using an array of "
                                             << bytes
                                             << "-bytes (bitstrm saves you "
                                             << double(bytes*8-bsize)/(bytes*8)*100.
                                             << "%)" << endl;

  switch(bytes){
  case 1:
    {
      vector<unsigned char> c(graph_size);
      generate_measure_and_check(c, bsize);
      break;
    }
    break;
  case 2:
    {
      vector<uint16_t> c(graph_size);
      generate_measure_and_check(c, bsize);
      break;
    }
  case 4:
    {
      vector<uint32_t> c(graph_size);
      generate_measure_and_check(c, bsize);
      break;
    }
  case 8:
    {
      vector<uint64_t> c(graph_size);
      generate_measure_and_check(c, bsize);
      break;
    }
  default:
    assert("unexpected condition");
  }
  
  return 0;
}
