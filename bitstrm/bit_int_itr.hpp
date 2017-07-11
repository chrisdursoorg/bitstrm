// bit_int_itr.hpp


// IMPLEMENTATION NOTES:
// I tried to adapt iterator_facade or iterator_adaptor.  Looks like iterator_adaptor is  is the right way to go, but I just could not seem to get it to come together.

//
// Quick easy iterator facade for arbitrary bit integers
//
//  Example:
//  // assuming that p0 is a bitstrm to an array of 13 bit integers [0] would print the 0th value and [1] would print (and change) that value to 42
//
//     assert( bitSz == 13 && "We have a mix of lvalue and dynamic, fine as long as they are size equivalent"); 
//     bit_int_itr<13,ureg> a(p0);
//     cout << *a << endl;       // [0]
//     cout << (*a=42) << endl;  // [1]

#ifndef BIT_INT_ITR_HPP_
#define BIT_INT_ITR_HPP_

#include "bitstrm/bitstrm.hpp"

#include <cassert>
#include <iterator>
#include <iostream>
#include <boost/iterator/iterator_facade.hpp>
#include <boost/static_assert.hpp>

namespace bitint {

  /// Cx02/98 approach in addition to swap need to do other std::algos, the pattern is 
  /// vector<bool>::reference, truly standards uglyness
    
  template<typename SIGN_UNSIGN>
  struct _bit_int_reference
  {
      
    _bit_int_reference(unsigned bitsz, bitstrm p): p_(p), bitsz_(bitsz){}
    _bit_int_reference(){}

    operator SIGN_UNSIGN() const {
      // std::cout << "READ: ";  print(std::cout); std::cout << std::endl; 
      return read();
    }

    _bit_int_reference&
    operator=(SIGN_UNSIGN __x) {
      // std::cout << "WRITE: ";  print(std::cout); std::cout << "<<" << __x << std::endl; 
      write(__x);
      return *this;
    }

    inline  _bit_int_reference&
    operator=(const _bit_int_reference& __x){ 
      // std::cout << "RWRITE: ";  print(std::cout); std::cout << "<<" << __x.read() << std::endl; 
      write(__x.read());
      return *this;
    }

    bool
    operator==(const _bit_int_reference& __x) const
    { return p_ == __x.p_; }

    bool
    operator<(const _bit_int_reference& __x) const
    { return p_ < __x.p_; }

    void print(std::ostream& out)const{
      p_.print(out); out << " " << read();
    }

  private:
    bitstrm p_;  
    unsigned bitsz_;
    inline SIGN_UNSIGN read()const           { return p_.read_as<SIGN_UNSIGN>(bitsz_); }
    inline void        write(SIGN_UNSIGN __x){        p_.write(bitsz_, __x); } 

  }; // struct _bit_int_reference


  // swap
  //
  // swaps deep down to the undrlying bit-integer
  template<typename _S>
  void swap(_bit_int_reference<_S> lhs, _bit_int_reference<_S> rhs){
    _S t = lhs;
    lhs = (_S)rhs;
    rhs = t;
  }

  template<typename _S>
  void swap(_S& lhs, _bit_int_reference<_S> rhs){
    _S tmp = lhs;
    lhs = rhs;
    rhs = tmp;
  }

  template<typename _S>
  void swap(_bit_int_reference<_S> lhs, _S& rhs){
    _S tmp = lhs;
    lhs = rhs;
    rhs = tmp;
  }


  // boost_facade_style_itr
  //
  // boost_facade_style_itr template has 2 interfaces, first it works with a static 
  // BSIZE defined, in which case the object is defined with the ctor without bsize.
  // Second allowing the BSIZE to default to -1, allows/requires the dynamic definition
  // of bsize
  //
  // Limitations: reasonably, different BSIZE iterators are not comparable, unless 
  // BSIZE == -1 where itrator will compile but comparison will not be defined when
  // lhs.m_bsize != rhs.m_bsize.  Signal will be thrown only on DEBUG build
  template<typename SIGNED_UNSIGNED, int BSIZE = -1> 
  class boost_facade_style_itr: public boost::iterator_facade<
    boost_facade_style_itr<SIGNED_UNSIGNED, BSIZE>
    , SIGNED_UNSIGNED
    , boost::random_access_traversal_tag
    , _bit_int_reference<SIGNED_UNSIGNED>
    >
  {
  public:
    typedef boost_facade_style_itr my_type;
    typedef SIGNED_UNSIGNED        value_type;

    // ctor - see above for qualifications
    boost_facade_style_itr(const bitstrm& beg)
      : m_bsize(BSIZE), m_beg_value(beg) {
      BOOST_STATIC_ASSERT_MSG(BSIZE >= 0, "see notes on boost_facade_style_itr, using this ctor you must have BSIZE defined as a positive integer or zero");
    }
    boost_facade_style_itr(const bitstrm& beg, int bsize)
    : m_bsize(bsize), m_beg_value(beg) {
      BOOST_STATIC_ASSERT_MSG(BSIZE == -1, "see notes on boost_facade_style_itr, using this ctor you must have BSIZE defined as -1");
      assert(bsize >= 0);
    }

  private:

    friend class boost::iterator_core_access;
    int     m_bsize;
    bitstrm m_beg_value;

    _bit_int_reference<value_type> dereference()const{ return _bit_int_reference<value_type>(m_bsize, m_beg_value); } 
    bool                           equal(const my_type& rhs)const{
      assert(m_bsize == rhs.m_bsize && "comparison between dissimilar iterators attempted");
      return m_beg_value == rhs.m_beg_value; 
    }
    void                           increment(){ m_beg_value += m_bsize; }
    void                           decrement(){ m_beg_value -= m_bsize; }
    void                           advance(reg n) { m_beg_value += m_bsize*n;}
    reg                            distance_to(const my_type& rhs)const{ return (rhs.m_beg_value - m_beg_value)/m_bsize; }
  };
  
  template<int BSIZE, typename SIGNED_UNSIGNED>
  using bit_int_itr =  boost_facade_style_itr<SIGNED_UNSIGNED, BSIZE>; 

  template<typename SIGNED_UNSIGNED>
  using dbit_int_itr =  boost_facade_style_itr<SIGNED_UNSIGNED, -1>; 

}  // namespace bitint
#endif // BIT_INT_ITR_HPP_
