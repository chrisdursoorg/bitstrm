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
    
    // alloced_bref
    //
    // allocate at least bsize + pre_reserved space, with pre_reserved bits
    // accessible via pre(), all other operations reference bref following 
    // reserved bits
    alloced_bref(ureg bsize, ureg pre_reserved = 0): m_reserved(pre_reserved){allocate(bsize);}
    alloced_bref(): m_reserved(0) {allocate(0);}
    alloced_bref(alloced_bref&& rhs) = default;
    alloced_bref& operator=(alloced_bref&& rhs) = default;
    
    
    // set_reserved
    //
    // sets the reserved mark, with no alteration of the underlying memory
    void set_prereserve(ureg pre_reserved){ m_reserved = pre_reserved;}
    
    // swap
    //
    // non allocating constant time swap
    void swap(alloced_bref& rhs){
      std::swap(static_cast<bref&>(*this), static_cast<bref&>(rhs));
      std::swap(m_reserved, rhs.m_reserved);
      m_buf.swap(rhs.m_buf);
    }
    
    // allocate
    //
    // allow for late initialization as is often useful for build up of substreams
    void allocate(ureg bsize) {
      bsize += m_reserved;
      m_buf.resize(uregs(bsize));
      m_bytesize = bref::_chars(bsize);
      reset();
    } 
    
    // reset
    //
    // resets position to begining of buffer following reserve.
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
    // includes 
    void zero(){fill(m_buf.begin(), m_buf.end(), 0); }

    // data
    //
    // returns a pointer to the begining of internal buffer for io
    // operations.  Result invalidated on allocate.
    const char* data()const{ return reinterpret_cast<const char*>(&m_buf.front()); }
          char* data()     { return reinterpret_cast<      char*>(&m_buf.front()); }
    
    // bytesize
    //
    // the underlying 'whole byte' size as indicated with bsize
    ureg bytesize()const{ return m_bytesize; }

    
  private:
    std::vector<ureg> m_buf;
    ureg              m_bytesize;
    ureg              m_reserved;
  };

} 


#endif // def'd ALLOCED_BREF_HPP_
