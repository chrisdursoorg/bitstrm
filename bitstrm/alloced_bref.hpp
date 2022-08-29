#ifndef ALLOCED_BREF_HPP_
#define ALLOCED_BREF_HPP_

#include "bitstrm/reg.hpp"
#include "bitstrm/bref.hpp"

#include <iosfwd>
#include <vector>

namespace bitstrm {  

  // alloced_bref
  //
  // a heap re/allocate for bitstrm
  //

  class alloced_bref : public bref {
  public:
     
    // alloced_bref
    //
    // allocate at least bsz bits
    alloced_bref(ureg bsz){allocate(bsz);}
    alloced_bref(){allocate(0);}
    alloced_bref(alloced_bref&& rhs) = default;
    alloced_bref& operator=(alloced_bref&& rhs) = default;
        
    // swap
    //
    // non allocating swap
    void swap(alloced_bref& rhs){
      std::swap(static_cast<bref&>(*this), static_cast<bref&>(rhs));
      m_buf.swap(rhs.m_buf);
    }
    
    // allocate
    //
    // allocate at least new_bsize bits invalidates any prior bref to this
    // memory, reset bref to beginning of this buffer
    void allocate(ureg new_bsize) {
      m_bsize = new_bsize;
      m_buf.resize(uregs(m_bsize));
      reset();
    } 
    
    // reset
    //
    // resets position to begining of buffer
    void reset(){
      this->bref::operator=(bref(data()));
    }

    // zero
    //
    // fills the underlying allcoated memory to zero
    // includes header section
    void zero(){fill(m_buf.begin(), m_buf.end(), 0); }

    // data
    //
    // returns a pointer to the begining of internal buffer for io
    // operations
    const char* data()const{ return reinterpret_cast<const char*>(m_buf.data()); }
          char* data()     { return reinterpret_cast<      char*>(m_buf.data()); }
    
    // bsize/bytesize
    //
    // the underlying whole true or 'whole byte' size
    ureg bsize()const{ return m_bsize; }
    ureg bytesize()const{ return (m_bsize + CHAR_BIT-1)/CHAR_BIT; }
    
    
  private:
    std::vector<ureg> m_buf;
    ureg              m_bsize;
  };


  // copy
  //
  // copies the content of src (ignoring current position)  into dest
  // where dest must be sized to at least src.bsize()
  inline bref copy(alloced_bref const& src, bref dest){
    bref const bbeg(const_cast<char*>(src.data()));
    return bitstrm::copy(bbeg, bbeg + src.bsize(), dest);
  }
  
} 


#endif // def'd ALLOCED_BREF_HPP_
