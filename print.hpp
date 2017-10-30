// print.hpp
//
//
// iostream et. al. adds considerable expense to compilation hence can
// be isolated here to be included explicitely in just the necessary
// .cpp or hpp files

#ifndef BITSTRM_PRINT_HPP
#define BITSTRM_PRINT_HPP

#include <iostream>
#include <sstream>
#include "bitstrm/bref.hpp"

namespace bitstrm {

  inline std::ostream&
  bref::print(std::ostream& dest)const{    
    std::ios_base::fmtflags restore = dest.setf(std::ios::hex, std::ios::basefield);
    dest << std::hex << m_addr;
    dest << " +" << std::dec << m_off;
    dest.setf(restore);
    return dest;
  }

  #if 0
  inline std::string
  bref::print()const{
    std::stringstream str;
    print(str);
    return str.str();
  }
  #endif
  
}

#endif // def'd BITSTRM_PRINT_HPP
