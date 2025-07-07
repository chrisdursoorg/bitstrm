// sort_10_bit.cpp
//

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

  const size_t testSize = 1024*1024;

  stringstream str;
  str << "sort " << testSize << " signed random elements of size 10 bits";
  TimeFixture fixture(str.str());
  vector<reg>             testSet64(testSize);  
  vector<reg>  ::iterator c64(testSet64.begin());
  vector<short>           testSet16(testSize);
  vector<short>::iterator c16(testSet16.begin());
  const reg               ten_bit_mask((1<<10) -1);
  alloced_bref            buf(testSize*10);
  mutable_bint_itr<reg>   b0(buf, 10);
  auto                    bc(b0);
  {
    stringstream str;
    str << "created " << testSize << " random elements storing as reg, short and 10 bit reg";
    TimeFixture fixture(str.str());
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
  auto be(bc);
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

