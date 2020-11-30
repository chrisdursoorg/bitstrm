// example.cpp

#include <iostream>
#include <array>
#include "bitstrm/utility.hpp"
#include "bitstrm/alloced_bref.hpp"
#include <vector>
#include <cassert>

using namespace std; 
using namespace bitstrm;

// should_resize
//
// see inline code comments below, returns should write a Size block
bool should_resize(unsigned long long value, unsigned& cur_bits, unsigned& wasted_bits){
  unsigned m = min_bits(value);
  if(wasted_bits >= 6 || cur_bits < m ){
    cur_bits = m;
    wasted_bits = 0;
    return true;
  }
  wasted_bits += (m - cur_bits);
  return false;
}

int main(int /*argc*/, const char** /*argv*/){
  
  cout << 
    "Simple example where we we will store and then recall some binary values.\n"
    "The values will generally be near 0 and will all be unsigned\n" << endl << endl;
  cout << 
    "This codec is made mostly for simplicity, rather than utility.\n"
    "we could for instance recognize that first Value 1st bit is\n"
    "redundant with Size magnitude and do a better job deciding where\n"
    "to have an extra pad 0 bit or a new Size block." << endl << endl;
  
  cout << 
    "Decode: [[Size]<Value><Control>], if control is 1 read [Size], [Size] is 6 bits\n"
    "Code:   [[Size]<Value><Control>], adopt a Size to hold Value, retain Size until until\n"
    "1) Size cannot hold Value (not enough bits), or 2) total wasted bits (preceeding zeros)\n"
    "is greater than 6" << endl << endl;
  cout << "example data: " << endl;
  
  array<unsigned long long, 32> data = {{
      12, 0, 1, 2, 3, 4, 5, 1234567, 
      922338962904924173U, 65, 82, 23, 11, 1, 1, 1, 
      890, 54, 0, 0, 0, 0, 17, 232902332, 
      89, 2398, 12, 12, 12, 558, 235, 238923}}; 
  
  for(auto a : data)
    cout << a << ",\n"; 
  
  // two passes through data, first to determine how many bits, second to code those bits
  
  unsigned total_bits  = 0;
  unsigned wasted_bits = 6;
  unsigned cur_bits = 0;
  bool first = true;
  for(auto a : data){
    if(!first)
      ++total_bits;
    if(should_resize(a, cur_bits, wasted_bits))
      total_bits +=  6;

    total_bits += cur_bits;   
    first = false;
  }
  
  cout << "total_bits: " << total_bits << " bytes: " 
       << bref::_chars(total_bits) << " bits/number: " 
       << double(total_bits)/data.size() 
       << " savings: " << (1. - double(bref::_chars(total_bits))/(data.size()*sizeof(unsigned long long)))*100. 
       << "%" << endl;

  // second pass through the data, now code
  
  alloced_bref cur(total_bits);
  bref beg = cur;
  
  cur_bits = 0;
  wasted_bits = 6;
  first = true;
  for(auto a : data){
    if(should_resize(a, cur_bits, wasted_bits)){
      if(!first)
	cur.iwrite(1, 1);
      cur.iwrite(cur_bits, 6);
    }
    else {
      cur.iwrite(0, 1);
    }
    cur.iwrite(a, cur_bits);
    first = false;
  }
  
  cout << "verify: " << cur - beg << " bits used equals: " << total_bits << " bits reserved" << endl;
  assert( (cur - beg) == total_bits);
  
  // verify that we can decode all the original values
  cur_bits = 0;
  wasted_bits = 6;
  first = true;
  cur.reset();
  bool read_bit_size = true;
  for(auto a : data){
    if(!read_bit_size)
      read_bit_size = cur.iread<ureg>(1);
    if(read_bit_size)
      cur_bits = cur.iread<ureg>(6);
    ureg restore = cur.iread<ureg>(cur_bits);
    cout << "restore: " << restore << " matches: " << a <<  endl;
    assert(restore == a);
    read_bit_size = false;
  }
    
  return 0;
}
