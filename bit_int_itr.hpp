// bit_int_itr.hpp


// bit_int_itr effectively converts a bitstrm (or substream) into an
// array of BSIZE SIGN_UNSIGN(ed) integers.  It works nearly
// seamlessly (in the same manner as std::vector<bool> which under the
// covers specializes its bit-wise storage) to present a reference
// wrapper (in our case we call it _bit_int_rereference) to facilitate
// getting and setting indirectly the state under the management of
// the bitstrm library.
//
// Performance suffers a little bit (a factor of approximately 3), and
// there are some subtle potential problems (e.g. *my_itr dereferences
// to a wrapper not the true object) but as with a number of iterator
// facades, adaptors and specializations the flexibility often exceeds
// the risk and performance costs associated.
//
// bitstrm p0(13 * 1);
// bit_int_itr<13,ureg> a(p0);

// *a = 40;
// cout << *a << endl;             // 40
// cout << (*a=42) << endl;        // 42
// cout << (*(--(++a)))  << endl;  // 42


#ifndef BIT_INT_ITR_HPP_
#define BIT_INT_ITR_HPP_

#include "bitstrm/alloced_bref.hpp"

#include <cassert>
#include <iterator>
#include <iostream>
#include <boost/iterator/iterator_facade.hpp>
#include <boost/static_assert.hpp>

namespace bitstrm {

  template<typename SIGN_UNSIGN>
  struct _bit_int_reference
  {
      
    _bit_int_reference(unsigned bitsz, bref p): p_(p), bitsz_(bitsz){}
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
    operator<(const _bit_int_reference& __x) const
    { return read() < __x.read(); }

    
    void print(std::ostream& out)const{
      p_.print(out); out << " " << read();
    }


  private:
    bref p_;  
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
  // boost_facade_style_itr template has 2 interfaces, first it works
  // with a static BSIZE defined, in which case the object is defined
  // with the ctor without bsize.  Second allowing the BSIZE to
  // default to -1, allows/requires the dynamic definition of bsize
  //
  // Limitations: reasonably, different BSIZE iterators are not
  // comparable, unless BSIZE == -1 where itrator will compile but
  // comparison will not be defined when lhs.m_bsize != rhs.m_bsize,
  // instead an assert will occur in DEBUG build
  
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
    boost_facade_style_itr(const bref& beg)
      : m_bsize(BSIZE), m_beg_value(beg) {
      BOOST_STATIC_ASSERT_MSG(BSIZE >= 0,
                              "see notes on boost_facade_style_itr, using this "
                              "ctor you must have BSIZE defined as zero or a "
                              "positive integer or zero");
    }
    boost_facade_style_itr(const bref& beg, int bsize)
    : m_bsize(bsize), m_beg_value(beg) {

      BOOST_STATIC_ASSERT_MSG(BSIZE == -1, "see notes on boost_facade_style_itr"
                              ", using this ctor you must have BSIZE defined as"
                              "-1");
      assert(bsize >= 0);
    }

  private:

    friend class boost::iterator_core_access;
    int     m_bsize;
    bref    m_beg_value;

    _bit_int_reference<value_type> dereference()const{
      return _bit_int_reference<value_type>(m_bsize, m_beg_value);
    }
    bool                           equal(const my_type& rhs)const{
      assert(m_bsize == rhs.m_bsize &&
             "comparison between dissimilar iterators attempted");
      return m_beg_value == rhs.m_beg_value; 
    }

    void                           increment(){ m_beg_value += m_bsize; }
    void                           decrement(){ m_beg_value -= m_bsize; }
    void                           advance(reg n) { m_beg_value += m_bsize*n;}
    reg                            distance_to(const my_type& rhs)const{
      return (rhs.m_beg_value - m_beg_value)/m_bsize;
    }
  };
  
  // bit_int_itr
  //
  // e.g. set up an 10 bit table of consequtive integers [0, 1024) in
  // a random order
  //
  // bitstrm buf(10*1024);
  // bit_int_itr<10, ureg> beg(buf);
  // bit_int_itr<10, ureg> end(beg + 1024);
  // unsigned i = 0;
  // for_each(beg, end, [&i](unsigned v){ v = i++;});
  // random_shuffle(beg, end);

  template<int BSIZE, typename SIGNED_UNSIGNED>
  using bit_int_itr =  boost_facade_style_itr<SIGNED_UNSIGNED, BSIZE>; 

  // dbit_int_itr
  //
  // e.g. set up an k-bit table of consequtive integers [0, 2**k) in
  // a random order
  //
  // bitstrm buf(10*1024);
  // dbit_int_itr<10, ureg> beg(buf);
  // dbit_int_itr<10, ureg> end(beg + 1024);
  // unsigned i = 0;
  // for(auto cur = beg; cur != end; ++cur){ *cur = i++; }
  // random_shuffle(beg, end);


  template<typename SIGNED_UNSIGNED>
  using dbit_int_itr =  boost_facade_style_itr<SIGNED_UNSIGNED, -1>; 

}  // namespace 
#endif // BIT_INT_ITR_HPP_
