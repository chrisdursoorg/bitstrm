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
  // maintains a seperate 'reserved' section 
  //
  // pre()[reserved-bits]|{this->bref}
  
  class alloced_bref : public bref {
  public:
     
    // alloced_bref
    //
    // allocate at least main_bsize + pre_reserved space, sets this
    // location following reserved bits

    alloced_bref(ureg main_bsize, ureg pre_reserved = 0)
      : m_reserved(pre_reserved){allocate(main_bsize);}
    alloced_bref(): m_reserved(0) {allocate(0);}
    alloced_bref(alloced_bref&& rhs) = default;
    alloced_bref& operator=(alloced_bref&& rhs) = default;
    
    
    // set_reserve
    //
    // sets the reserved bsize
    void set_reserve(ureg reserved){ m_reserved = reserved;}

    // get_reserved
    //
    // gets the reserved bsize
    ureg get_reserve()const { return m_reserved; }
    
    // swap
    //
    // non allocating swap
    void swap(alloced_bref& rhs){
      std::swap(static_cast<bref&>(*this), static_cast<bref&>(rhs));
      std::swap(m_reserved, rhs.m_reserved);
      m_buf.swap(rhs.m_buf);
    }
    
    // allocate
    //
    // allocate or reallocate bsize + reserved bits
    void allocate(ureg bsize) {
      m_bsize = bsize + m_reserved;
      m_buf.resize(uregs(m_bsize));
      reset();
    } 
    
    // reset
    //
    // resets position to begining of buffer *following* reserve
    void reset(){
      this->bref::operator=(pre() + m_reserved);
    }

    // pre
    //
    bref pre(){
      return bref(data());
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
    ureg              m_reserved;
  };

} 


#endif // def'd ALLOCED_BREF_HPP_
