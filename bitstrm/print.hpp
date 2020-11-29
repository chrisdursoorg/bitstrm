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


  inline
  std::ostream&
  bref::print()const{
    return print(std::cout);
  }
  
  inline
  std::ostream&
  bref::print(std::ostream& dest)const{    

    if(m_addr == nullptr)
      dest << "0xNullptr";
    else {
      std::ios_base::fmtflags restore = dest.setf(std::ios::hex, std::ios::basefield);
      dest << std::hex << m_addr;
      dest << " +" << std::dec << m_off;
      dest.setf(restore);
    }
    return dest;
  }
  
  inline
  std::ostream&
  print(std::ostream& out, bref cur, bref end){

    out << 'b';
    for(; cur != end;)
      out << cur.iread<ureg>(1);
      
    return out;
  }
  
  
}

#endif // def'd BITSTRM_PRINT_HPP
