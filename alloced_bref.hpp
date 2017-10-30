#ifndef ALLOCED_BREF_HPP_
#define ALLOCED_BREF_HPP_

#include "bitstrm/reg.hpp"
#include "bitstrm/bref.hpp"

#include <iosfwd>
#include <vector>

namespace bitstrm {  

  // alloced_bref
  //
  // allocator for convienince, either allocate a fixed size at
  // intialization or else call allocate method to reset() ref value
  // at beginning of newly allocated memory buffer.  
  
  class alloced_bref : public bref {
  public:

    alloced_bref(ureg bsize): m_buf(uregs(bsize)){ reset(); }
    alloced_bref(){}

    // swap
    //
    // non allocating constant time swap
    void swap(alloced_bref& rhs){
      std::swap(static_cast<bref&>(*this), static_cast<bref&>(rhs));
      m_buf.swap(rhs.m_buf);
    }
    
    // allocate
    //
    // allow for late initialization as is often useful for build up of substreams
    void allocate(ureg bsize) { m_buf.resize(uregs(bsize)); reset();} 
    
    // reset
    //
    // resets position to begining of buffer.
    void reset(){
      this->bref::operator=(bref(&*m_buf.begin()));
    }

  private:
    std::vector<ureg> m_buf;
  };

} 


#endif // def'd ALLOCED_BREF_HPP_
