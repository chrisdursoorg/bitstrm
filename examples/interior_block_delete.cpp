// interior_block_delete.cpp
//
// SENARIO
//
// Given an allocated bitstrm have deleted sections consolidate remaining
// sections such that data may be appended to the end

#include <cstdint>
#include "bitstrm/reg.hpp"
#include "bitstrm/alloced_bref.hpp"
#include "bitstrm/utility.hpp"
#include "bitstrm/TimeFixture.hpp"
#include "bitstrm/bint_itr.hpp"

using namespace bitstrm;
using namespace std;


const long test_size = 1024*1024*512;

unsigned TimeFixture::s_timer_number;

template<class I>
I cycle16(reg v) {return is_signed_v<I> ? v % 16 - 8 : v % 16; }

template<class I>
void vector_approach(){

  vector<I>  b(test_size);

  reg i = 0;
  generate(b.begin(), b.end(), [&i](){ return cycle16<I>(i++);});

  string msg("shift_left and check ");
  if(not is_signed_v<I>)
    msg += "un";
  msg += "signed 8 bit";
  TimeFixture consolidate(msg.c_str());

  ureg remove  = b.size()*11/100;
  ureg shifts = 0;
  ureg total_bits_moved = 0;
  ureg v0 = 0;
  
  while(remove > 128){
      v0 += remove;
      // cout << '[' << v0 << ", " << b.size() << ')'  << endl;
      auto end = copy(b.begin() + remove, b.end(), b.begin());
      b.resize(b.size() - remove);
	
      ureg vx = v0;
      for(auto cur = b.begin(); cur != end; ++cur, ++vx)
	if(cycle16<I>(vx) != *cur)
	  cout << "error in consolidate" << endl;
	  
      total_bits_moved += b.size()*8;
      ++shifts;
      remove = b.size()*11/100;
    }
    cout << "with " << shifts << " shifts moved a total of "
	 << total_bits_moved << " bits "
	 << double(total_bits_moved + 7)/(8*1024*1024)
	 << " m bytes." << endl;
}


template<class I>
void bitstrm_approach(){

  unsigned bwide(4);
  alloced_bref b(test_size*bwide);
  reg i = 0;
  generate(mutable_bint_itr<I>(b, bwide),
	   mutable_bint_itr<I>(b, bwide) + test_size,
	   [&i](){ return cycle16<I>(i++);});

  string msg("shift_left and check ");
  if(not is_signed_v<I>)
    msg += "un";
  msg += "signed 4 bit";
  TimeFixture consolidate(msg.c_str());

  ureg remove  = b.bsize()/bwide*11/100;
  ureg shifts = 0;
  ureg total_bits_moved = 0;
  ureg v0 = 0;
  
  while(remove > 128){
      v0 += remove;
      // cout << '[' << v0 << ", " << b.size() << ')'  << endl;
      auto end = bitstrm::copy(b.bbegin() + remove*bwide, b.bend(), b.bbegin());
      b.resize(b.bsize() - remove*bwide);
	
      ureg vx = v0;
      for(bref cur = b; cur != end; ++vx)
	if(cycle16<I>(vx) != cur.iread<I>(bwide))
	  cout << "error in consolidate" << endl;
	  
      total_bits_moved += b.bsize();
      ++shifts;
      remove = b.bsize()/bwide*11/100;
    }
    cout << "with " << shifts << " shifts moved a total of "
	 << total_bits_moved << " bits "
	 << double(total_bits_moved + 7)/(8*1024*1024)
	 << " m bytes." << endl;
}

int main(int /*argc*/, char** /*argv*/){

  vector_approach<int8_t> ();
  vector_approach<uint8_t>();
  bitstrm_approach<reg>   ();
  bitstrm_approach<ureg>  ();
  
  cout << endl;  
  return 0;
}
