#ifndef ALLOCED_BREF_HPP_
#define ALLOCED_BREF_HPP_

#include <bitstrm/reg.hpp>
#include <bitstrm/bref.hpp>
#include <bitstrm/bitstrm.hpp>

#include <iosfwd>
#include <vector>
#include <algorithm>

namespace bitstrm {  

  // alloced_bref
  //
  // hybrid of a buffer and a bref (e.g. can reset back to bbegin) keeping
  // track of [bbegin(), bend()) bitstrm.
  //
  // Conforms to std::vector buffer invalidation.

  class alloced_bref : public bref {
  public:
     
    // alloced_bref
    //
    // allocate at least bsz bits
    alloced_bref(ureg bsz){resize(bsz);}
    alloced_bref(){resize(0);}
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
    void resize(ureg new_bsize) {
      m_bsize = new_bsize;
      m_buf.resize(uregs(m_bsize));
      reset();
    } 
    
    // reset
    //
    // resets position to beginning of buffer
    void reset(){
      this->bref::operator=(bref(data()));
    }

    // zero
    //
    // fills the underlying allocated memory to zero
    // includes header section
    void zero(){std::fill(m_buf.begin(), m_buf.end(), 0); }

    // data
    //
    // returns a pointer to the beginning of internal buffer for io
    // operations
    [[nodiscard]] reg const* data()const{ return m_buf.data(); }
    [[nodiscard]] reg*       data()     { return m_buf.data(); }
    
    // bsize/bytesize
    //
    // the underlying whole true or 'whole byte' size
    [[nodiscard]] ureg bsize()const{ return m_bsize; }
    [[nodiscard]] ureg bytesize()const{ return (m_bsize + CHAR_BIT-1)/CHAR_BIT; }

    // bbegin/bend
    //
    // returns the 'curent' position and the 'end' position (as initially
    // allocated
    [[nodiscard]] bref bbegin(){ return bref(data()); }
    [[nodiscard]] bref bend(){ return bbegin() + bsize(); }

    // print
    //
    // prints contents of this bitstrm (include and see print.hpp for details)
    // note bref::print prints address and these member functions print value
    // returns std::cout/out
    std::ostream& print()const;
    std::ostream& print(std::ostream& out)const;
    std::ostream& print(std::ostream& out, unsigned prefsuf)const;
    
  private:

    [[nodiscard]] bref const bbegin()const{ return bref(const_cast<reg*>(data()));}
    [[nodiscard]] bref const bend()const{ return bbegin() + bsize(); }
    
    std::vector<reg>  m_buf;
    ureg              m_bsize;
  };

  // invalid stream operator -- intentially undefined, cast to bref::operator<<
  // 
  std::ostream& operator<<(std::ostream& lhs, alloced_bref const& rhs);
  
  // copy
  //
  // copies the content of src (ignoring current position) into destination
  inline bref copy(alloced_bref const& _src, bref destination){
    bref src(_src);
    return bitstrm::copy(src, src + reg(_src.bsize()), destination);
  }
  
} 


#endif // def'd ALLOCED_BREF_HPP_
