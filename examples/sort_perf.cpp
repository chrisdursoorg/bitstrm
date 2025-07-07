// sort_perf.cpp

#include <iostream>
#include <ctime>
#include <vector>

#include "bitstrm/reg.hpp"
#include "bitstrm/alloced_bref.hpp"
#include "bitstrm/utility.hpp"
#include "bitstrm/bint_itr.hpp"
#include "bitstrm/TimeFixture.hpp"
#include <sstream>


using namespace bitstrm;
using namespace std;

unsigned bitstrm::TimeFixture::s_timer_number = 0;

int main(int /*argc*/ , const char** /*argv*/){

  {
    const size_t testSize = 8*1024*1024;
    stringstream str;
    str << "sort " << testSize << " unsigned random elements of size 10 bits";
    TimeFixture fixture(str.str().c_str());
    vector<ureg>                     testSet64(testSize);  
    vector<ureg>          ::iterator c64(testSet64.begin());
    vector<unsigned short>           testSet16(testSize);
    vector<unsigned short>::iterator c16(testSet16.begin());
    const ureg ten_bit_mask((1<<10) -1);
    alloced_bref            buf(testSize *10);
    mutable_bint_itr<ureg>  b0(buf, 10);
    auto                    bc(b0);

    {
      stringstream str;
      str << "created " << testSize << " random elements storing as ureg, unsigned short and 10 bit ureg";
      TimeFixture fixture(str.str().c_str());
      for(vector<ureg>::iterator e = testSet64.end(); c64 < e; ++c64, ++c16, ++bc)
	*bc = *c16 = *c64 = (ureg)rand() & ten_bit_mask;
    }
    auto be(bc);
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
