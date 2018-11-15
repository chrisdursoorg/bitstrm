// sort_perf.cpp

#include <iostream>
#include <ctime>
#include <vector>

#include "bitstrm/reg.hpp"
#include "bitstrm/alloced_bref.hpp"
#include "bitstrm/utility.hpp"
#include "bitstrm/bit_int_itr.hpp"
#include "bitstrm/TimeFixture.hpp"
#include <boost/lexical_cast.hpp>


using namespace bitstrm;
using namespace std;

unsigned bitstrm::TimeFixture::s_timer_number = 0;

int main(int /*argc*/ , const char** /*argv*/){

  {
    const size_t testSize = 8*1024*1024; 
    TimeFixture fixture((string("sort ") + boost::lexical_cast<string>(testSize) + " unsigned random elements of size 10 bits").c_str());
    vector<ureg>                     testSet64(testSize);  
    vector<ureg>          ::iterator c64(testSet64.begin());
    vector<unsigned short>           testSet16(testSize);
    vector<unsigned short>::iterator c16(testSet16.begin());
    const ureg ten_bit_mask((1<<10) -1);
    vector<char> buf((testSize *10 + 7)/8);
    bit_int_itr<10,ureg>  b0(bref(&buf.front()));
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
}
