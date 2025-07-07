// print.hpp
//
//
// iostream et. al. adds considerable expense to compilation hence can
// be isolated here to be included explicitely in just the necessary
// .cpp or hpp files

#ifndef BITSTRM_PRINT_HPP
#define BITSTRM_PRINT_HPP

#include <iostream>
#include "bitstrm/bref.hpp"
#include "bitstrm/alloced_bref.hpp"
#include "bitstrm/bitstrm.hpp"

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
      dest << "0xNullptr +0";
    else {
      std::ios_base::fmtflags restore = dest.setf(std::ios::hex, std::ios::basefield);
      dest <<  m_addr;
      dest << " +" << std::dec << m_off;
      dest.setf(restore);
    }
    return dest;
  }
  
  // print
  //
  // prints to output a binary formatted string
  // presuf option allows for truncating with ellipsis mark 
  // return out or cout
  inline std::ostream&
  print(std::ostream& out, bref c, bref e, unsigned prefsuf){

  ureg     bsize   = e - c;
  
  for(unsigned i = 0 ;c != e; ++c, ++i)
    if( (i < prefsuf) or (i > (bsize - prefsuf)))
      out << c.read<ureg>(1);
    else if( i == prefsuf)
      out << "...";

  return out;
  }

  inline std::ostream&
  print(bref c, bref e, unsigned p){return print(std::cout, c, e, p); }
  inline std::ostream&
  print(bref c, bref e){return print(std::cout, c, e, (e-c)); }
  inline std::ostream&
  print(std::ostream& o, bref c, bref e){return print(o, c, e, (e-c)); }
  inline std::ostream&
  alloced_bref::print()const{ return print(std::cout);}
  inline std::ostream&
  alloced_bref::print(std::ostream& o)const{
    return print(o, bsize());
  }
  inline std::ostream&
  alloced_bref::print(std::ostream& o, unsigned prefsuf)const{
    return bitstrm::print(o, bbegin(), bend(), prefsuf);
  }

 inline std::ostream&
 print_human_bits(std::ostream& out, unsigned long long bits){
    
    unsigned long long  bytes = bits/8;

    if(bytes < 2){
      out << bits << " bits";
    } else if(bytes < 1024){
      unsigned long long whole_bytes = bits/8;
      out << whole_bytes << " bytes";
      unsigned long long remain = (bits - (whole_bytes*8));
      if(remain )
        out << " " << remain << " bits";
    } else if(bytes < 1024*1024){
      out << double(bits) / (8*1024) << " kbytes";
    } else if(bytes < 1024*1024*1024){
      out << double(bits) / (8*1024*1024) << " mbytes";
    } else {
      out << double(bits)/(double(8)*1024*1024*1024) << " gbytes";
    }
    
    return out;
  }


  
}

#endif // def'd BITSTRM_PRINT_HPP
