// misc_perf.cpp

#include <iostream>
#include <ctime>
#include <vector>

#include "bitstrm/reg.hpp"
#include "bitstrm/alloced_bref.hpp"
#include "bitstrm/utility.hpp"
#include "bitstrm/TimeFixture.hpp"
using namespace bitstrm;
using namespace std;


unsigned bitstrm::TimeFixture::s_timer_number = 0;

int main(int /*argc*/ , const char** /*argv*/){

  {
    cout << "In a ~128 m data point array, with a couple of initial gaps, copy array left over gap, with bitstrm copy" << endl;
    TimeFixture fixture("copy 128m");
    const size_t exact_quanity = 128*1024*1024-575;
    vector<ureg> input_128m(exact_quanity); // just a hair under 128M is where first numerical overflow occurs, hence 575
    typedef vector<ureg>::iterator itr_t;

    itr_t c = input_128m.begin()+2; // leave the first 2 registers or 128 bits undefined
    itr_t e = input_128m.end();

    for( ureg i = 0; c != e; ++c, ++i)
      *c = i;
  
    cout << "enumeration of 128m data points complete" << endl;

    bref dc(reinterpret_cast<char*>(&*input_128m.begin()));
    bref sc(reinterpret_cast<char*>(&*(input_128m.begin()+2)));
    bref se(reinterpret_cast<char*>(&*input_128m.end()));
    
    {
      TimeFixture copy_itself("copy itself", exact_quanity);
      copy(sc, se, dc);
    }

    cout << "128m data points bitwise copied" << endl;

    c = input_128m.begin();
    bool failed = false;
    for( ureg i = 0; c != (e-2); ++c, ++i)
      failed  |= (*c != i);
    
    cout << (failed ? "failed" : "successful") <<   " checking 128m data points" << endl;    
  }
  cout << endl << endl;
  {
    cout << "Performance test of 1 B signextend operations from 1K (9 bit depth integers) pool" << endl;
    TimeFixture fixture("1G signext test");
    const int spaceSize(1024);
    const int bits = 9;
    const unsigned mask((1<<bits) - 1);   // all bits
    int signedBit      (1 << (bits -1));  // signed bit hi
    int signedBitMask  (~signedBit);      // ! signed bit hi

    // create a sampleSpace
    vector<int> sampleSpace(spaceSize);
    for(int i = 0; i < spaceSize; ++i){
      int r = rand();
      sampleSpace[i] = r & mask;
      
      if(r & signedBit)
	sampleSpace[i] = -(sampleSpace[i] & signedBitMask);
    }
    cout << "created a sample of 1K data points" << endl;
    
    int midWayPoint(1<<(bits-1));
    vector<uint64_t> histogram(1<<bits,0);
    {
      TimeFixture fixture("1B signext", 1024*1024*1024);
      
      for(int i = 0; i < 1024*1024; ++i){
	vector<int>::const_iterator b(sampleSpace.begin());
	vector<int>::const_iterator e(sampleSpace.end());
	for( ; b != e; ++b){
	  int v  = bitstrm::signextend<signed int, bits>(*b & mask);
	  histogram[v + midWayPoint]++;
	}
      }
    }
  }
  cout << endl << endl;

}
