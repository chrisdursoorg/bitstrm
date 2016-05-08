// main.cpp
//

#include <iostream>
#include <ctime>

#include <vector>

#include "bitstrm/reg.hpp"
#include "bitstrm/bitstrm.hpp"
#include "bitstrm/utility.hpp"
#include "bitstrm/bit_int_itr.hpp"
#include <boost/lexical_cast.hpp>
using namespace bitint;
using namespace std;


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

    bitstrm dc(reinterpret_cast<char*>(&*input_128m.begin()));
    bitstrm sc(reinterpret_cast<char*>(&*(input_128m.begin()+2)));
    bitstrm se(reinterpret_cast<char*>(&*input_128m.end()));
    
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
	  int v  = bitint::signextend<signed int, bits>(*b & mask);
	  histogram[v + midWayPoint]++;
	}
      }
    }
  }
  cout << endl << endl;
  {
    const size_t testSize = 8*1024*1024; 
    TimeFixture fixture((string("sort ") + boost::lexical_cast<string>(testSize) + " unsigned random elements of size 10 bits").c_str());
    vector<ureg>                     testSet64(testSize);  
    vector<ureg>          ::iterator c64(testSet64.begin());
    vector<unsigned short>           testSet16(testSize);
    vector<unsigned short>::iterator c16(testSet16.begin());
    const ureg ten_bit_mask((1<<10) -1);
    vector<char> buf((testSize *10 + 7)/8);
    bit_int_itr<10,ureg>  b0(bitstrm(&buf.front()));
    bit_int_itr<10,ureg>  bc(b0);

    {
      TimeFixture fixture((string("created ") + boost::lexical_cast<string>(testSize) 
			   + " random elements storing as ureg, unsigned short and 10 bit ureg").c_str());
      for(vector<ureg>::iterator e = testSet64.end(); c64 < e; ++c64, ++c16, ++bc)
	*bc = *c16 = *c64 = (ureg)rand() & ten_bit_mask;
    }
    bit_int_itr<10,ureg> be(bc);
    bc = b0;
        
    {
      TimeFixture fixture("sorting as ureg", testSize);
      sort(testSet64.begin(), testSet64.end());
    }
    {
      TimeFixture fixture("sorting as unsigned short", testSize);
      sort(testSet16.begin(), testSet16.end());
    }
    {
      TimeFixture fixture("sorting as 10 bit integer", testSize);
      sort(bc, be);
    }
  }
  cout << endl << endl;
  {
    const size_t testSize = 8*1024*1024; 
    TimeFixture fixture((string("sort ") + boost::lexical_cast<string>(testSize) + " signed random elements of size 10 bits").c_str());
    vector<reg>             testSet64(testSize);  
    vector<reg>  ::iterator c64(testSet64.begin());
    vector<short>           testSet16(testSize);
    vector<short>::iterator c16(testSet16.begin());
    const reg               ten_bit_mask((1<<10) -1);
    vector<char> buf((testSize *10 + 7)/8);
    bit_int_itr<10,reg>     b0(bitstrm(&buf.front()));
    bit_int_itr<10,reg>     bc(b0);
    {
      TimeFixture fixture((string("created ") + boost::lexical_cast<string>(testSize) 
			   + " random elements storing as reg, short and 10 bit reg").c_str());
      for(vector<reg>::iterator e = testSet64.end(); 
	  c64 < e; 
	  ++c64, ++c16,  ++bc)
	{
	  reg orig = rand() & ten_bit_mask;
	  reg ten_bit_number = signextend<reg, 10>(orig);
	  // we don't care what the orig number is, just what is interpreted
	  // as when we take only ten bits of it.  This should approximate an
	  // even split of positive and negative numbers
	  *bc = *c16 = *c64 = ten_bit_number;
	}
    }
    bit_int_itr<10,reg> be(bc);
    bc = b0;
        
    {
      TimeFixture fixture("sorting as reg", testSize);
      sort(testSet64.begin(), testSet64.end());
    }
    {
      TimeFixture fixture("sorting as signed short", testSize);
      sort(testSet16.begin(), testSet16.end());
    }
    {
      TimeFixture fixture("sorting as 10 bit signed integer", testSize);
      sort(bc, be);
    }
  }


  
}

