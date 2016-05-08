#include "bitstrm/bitstrm.hpp"
#include "bitstrm/utility.hpp"
#include <iostream>
#include <sstream>

using namespace std;
using namespace bitint;

unsigned TimeFixture::s_timer_number = 0;

void 
bitstrm::bitstrm::print(std::ostream& dest)const{    
  std::ios_base::fmtflags restore = dest.setf(std::ios::hex, std::ios::basefield);
  dest << std::hex << addr_;
  dest << " +" << std::dec << off_;
  dest.setf(restore);
}

string
bitstrm::bitstrm::print()const{
  stringstream str;
  print(str);
  return str.str();
}
