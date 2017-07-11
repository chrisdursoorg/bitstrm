#include "bitstrm/bitstrm.hpp"
#include "bitstrm/utility.hpp"
#include <iostream>
#include <sstream>
#include <vector>

using namespace bitint;
using namespace std;


unsigned TimeFixture::s_timer_number = 0;

bitstrm::bitstrm(vector<char>& buf, ureg size){
  ureg alloc_bytes = chars(size);
  buf.resize(alloc_bytes);
  addr_ = reinterpret_cast<reg*>(&*buf.begin());
  off_ = 0;
}

std::ostream&
bitstrm::print(std::ostream& dest)const{    
  std::ios_base::fmtflags restore = dest.setf(std::ios::hex, std::ios::basefield);
  dest << std::hex << addr_;
  dest << " +" << std::dec << off_;
  dest.setf(restore);
  return dest;
}

string
bitstrm::print()const{
  stringstream str;
  print(str);
  return str.str();
}


