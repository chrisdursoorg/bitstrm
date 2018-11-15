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
#include <boost/type_traits/is_same.hpp>
#include <boost/type_traits.hpp>

namespace bitstrm {

  // _bit_int_reference
  //
  // The "ghost in the machine" for the iterator over the mutable
  // bitstrm.  As with vector<bool>::iterator the underylying non byte
  // alligned bits cannot be directly addressed in CPU so this proxy
  // object stands in for them so that you can "write" directly to
  // this object and it writes via the bref interface.
  //
  // The shortcoming of this approach is twofold.  First the dereferenced
  // type is NOT the underlying storage unit (i.e. a k bit integer).
  // Second there is overhead with this object that there would not be
  // with a reference to the underlying storage unit.
  //
  // The bit_int_base_itr allows for const implementation without these
  // shortcomings
  
  template<typename SIGN_UNSIGN>
  struct _bit_int_reference
  {      
    _bit_int_reference(unsigned bitsz, bref p)
      : p_(p), bitsz_(bitsz){
      read();
    }

    _bit_int_reference(const _bit_int_reference& rhs)
      : p_(rhs.p_),
        bitsz_(rhs.bitsz_),
        value_(rhs.value_)
      , old_value_(rhs.old_value_){}

    ~_bit_int_reference(){
      write();
    }

    operator SIGN_UNSIGN() const {
      return value_;
    }

    _bit_int_reference&
    operator++(){
      ++value_;
      return *this;
    }

    // just can't do this! even asuming that
    // _bit_int_reference is temporary,
    // the return object is *more* temporary, AND may
    // or may not transmute underlying value
    _bit_int_reference
    operator++(int) = delete;
    

    _bit_int_reference&
    operator--(){
      --value_;
      return *this;
    }

    _bit_int_reference&
    operator--(int){
      auto o = value_;
      operator=(++value_);
      return o;
    }
    
    _bit_int_reference&
    operator=(SIGN_UNSIGN __x) {
      value_ = __x;
      return *this;
    }

    // assignment
    //
    // as in act on underlying value
    _bit_int_reference&
    operator=(const _bit_int_reference& __x){
      value_ = __x.value_;
      return *this;
    }

    bool
    operator<(const _bit_int_reference& __x) const{
      return value_ < __x.value_;
    }

    void print(std::ostream& out)const{
      p_.print(out); out << ((value_ == old_value_) ? "  " : " *") << value_;
    }

  private:
    bref        p_;  
    unsigned    bitsz_;
    SIGN_UNSIGN value_;
    SIGN_UNSIGN old_value_;
    inline void  read()  { old_value_ = value_ = p_.read_as<SIGN_UNSIGN>(bitsz_); }
    inline void  write() { if(value_ != old_value_ ) p_.write(bitsz_, value_); } 

  public:
    // delete: for the time being, let us be as restrictive as we can
    _bit_int_reference(){}
  }; // struct _bit_int_reference

  // swap
  //
  // swaps deep down to the undrlying bit-integer
  template<typename _S>
  inline void swap(_bit_int_reference<_S> lhs, _bit_int_reference<_S> rhs){
    _S t = lhs;
    lhs = (_S)rhs;
    rhs = t;
  }

  template<typename _S>
  inline void swap(_S& lhs, _bit_int_reference<_S> rhs){
    _S tmp = lhs;
    lhs = rhs;
    rhs = tmp;
  }

  template<typename _S>
  inline void swap(_bit_int_reference<_S> lhs, _S& rhs){
    _S tmp = lhs;
    lhs = rhs;
    rhs = tmp;
  }

  template<class SIGNED_UNSIGNED>
  struct RefWrapper {
    typedef _bit_int_reference<SIGNED_UNSIGNED> ref_type;

    // RefWrapper(const ConstRefWrapper&){} intnetionally ommitted: not permitted
    
    ref_type gen_ref(int bsize, bref cur){
      return ref_type(bsize, cur);
    }
  };

  template<class SIGNED_UNSIGNED>
  struct ConstRefWrapper {
    typedef SIGNED_UNSIGNED const & ref_type;

    ConstRefWrapper(const ConstRefWrapper&){}
    ConstRefWrapper(const RefWrapper<SIGNED_UNSIGNED>&){} // intentionally uncluded
    ConstRefWrapper(){}


    ConstRefWrapper& operator=(const ConstRefWrapper&){ return *this; }
    ConstRefWrapper& operator=(const RefWrapper<SIGNED_UNSIGNED>&){ return *this;}
    
    ref_type gen_ref(int bsize, bref cur){
      return m_value = cur.read_as<SIGNED_UNSIGNED>(bsize);
    }

    constexpr static bool is_const_itr = true;

  private:
    SIGNED_UNSIGNED m_value;
  };
  
  
  // bit_int_base_itr
  //
  // bit_base_itr is specialized to const and non const variant and
  // contains 2 interfaces
  // * first static works with BSIZE : [0, c_register_bits] statically
  // bound, in which case the object is defined with the ctor without
  // bsize
  // * second setting the BSIZE => -1  allows/requires
  // the dynamic definition of bsize in the constructor
  //
  
  template<typename SIGNED_UNSIGNED, int BSIZE, typename REF_WRAPPER>
  class bit_int_base_itr: public boost::iterator_facade<
    bit_int_base_itr<SIGNED_UNSIGNED, BSIZE, REF_WRAPPER>
    , SIGNED_UNSIGNED
    , boost::random_access_traversal_tag
    , typename REF_WRAPPER::ref_type
    >
  {
  public:

    typedef bit_int_base_itr  my_type;
    typedef SIGNED_UNSIGNED   value_type;
    typedef REF_WRAPPER       wrapper_type;
    
    bit_int_base_itr(const bit_int_base_itr& rhs)
    {this->operator=(rhs);}

    // constructors, see above
    bit_int_base_itr(const bref& beg)
      : m_bsize(BSIZE), m_cur(beg) {
      static_static_asserts();
    }
    
    bit_int_base_itr(const alloced_bref& beg)
      : m_bsize(BSIZE), m_cur(beg) {
      static_static_asserts();
    }

    
    // note only some RHS_BIT_INT_ITR to my_type are permitted
    // (regulated in assignment operator)
    template<class RHS_BIT_INT_ITR>
    bit_int_base_itr(const RHS_BIT_INT_ITR& rhs)
      : m_bsize(rhs.m_bsize), m_cur(rhs.m_cur), m_ref(rhs.m_ref){}
    

    template<class RHS_BIT_INT_ITR>
    bit_int_base_itr& operator=(const RHS_BIT_INT_ITR& rhs){
      typedef boost::is_same<value_type, typename RHS_BIT_INT_ITR::value_type>
        same_value_type;
      BOOST_STATIC_ASSERT_MSG(same_value_type::value,
                              "assignment value_type does not match");
      assert( m_bsize == rhs.m_bsize && "bsize can be either static or dynamic");
      
      m_bsize = rhs.m_bsize;
      m_cur   = rhs.m_cur;
      m_ref   = rhs.m_ref;  // guards against non_const <- const
    }
    
    
    bit_int_base_itr(const bref& beg, int bsize)
      : m_bsize(bsize), m_cur(beg) {
      
      BOOST_STATIC_ASSERT_MSG(BSIZE == -1, "see notes on bit_int_base_itr"
                              ", using this ctor you must have BSIZE defined as"
                              "-1");
      assert(bsize > 0 && "validity check for bsize");
      assert(bsize <= (int)c_register_bits  && "dynamic bsize beyond the c_register_bits");
    }
    
  private:

    friend class boost::iterator_core_access;
    template<typename SU, int BS, typename RW>
    friend class bit_int_base_itr;

    int                 m_bsize;
    bref                m_cur;
    mutable REF_WRAPPER m_ref;
    
    typename REF_WRAPPER::ref_type dereference()const{
      return m_ref.gen_ref(m_bsize, m_cur);
    }
    
    bool                           equal(const my_type& rhs)const{
      assert(m_bsize == rhs.m_bsize &&
             "comparison between dissimilar iterators attempted");
      return m_cur == rhs.m_cur; 
    }

    void                           increment()   { m_cur += m_bsize; }
    void                           decrement()   { m_cur -= m_bsize; }
    void                           advance(reg n){ m_cur += m_bsize*n;}
    reg                            distance_to(const my_type& rhs)const{
      return (rhs.m_cur - m_cur)/m_bsize;
    }

    // let compiler check to see that reasonable static bsize is choosen
    static void static_static_asserts(){
      BOOST_STATIC_ASSERT_MSG(BSIZE > 0,
                              "see notes on bit_int_base_itr, using this "
                              "ctor you must have BSIZE defined as zero or a "
                              "positive integer or zero");
      
      BOOST_STATIC_ASSERT_MSG(BSIZE <= c_register_bits,
                              "static BSIZE beyond the c_register_bits"); 
    }
    
  };
  
  
  
  // bit_int_itr, bit_int_const_itr
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
  using bit_int_itr = bit_int_base_itr<SIGNED_UNSIGNED, BSIZE, RefWrapper<SIGNED_UNSIGNED >>;
  template<int BSIZE, typename SIGNED_UNSIGNED>
  using bit_int_citr = bit_int_base_itr<SIGNED_UNSIGNED, BSIZE, ConstRefWrapper<SIGNED_UNSIGNED >>;

  // dbit_int_itr, dbit_int_const_itr
  //
  // e.g. set up an k-bit table of consequtive integers [0, 2**k) in
  // a random order
  // int k = 10;
  // bitstrm buf(ten*1024);
  // dbit_int_itr<ureg> beg(buf, k);
  // dbit_int_itr<ureg> end(beg + (1 << k), k);
  // unsigned i = 0;
  // for(auto cur = beg; cur != end; ++cur){ *cur = i++; }
  // random_shuffle(beg, end);

  template<typename SIGNED_UNSIGNED>
  using dbit_int_itr =  bit_int_base_itr<SIGNED_UNSIGNED, -1, RefWrapper<SIGNED_UNSIGNED> >;
  template<typename SIGNED_UNSIGNED>
  using dbit_int_citr =  bit_int_base_itr<SIGNED_UNSIGNED, -1, ConstRefWrapper<SIGNED_UNSIGNED> >; 

}  // namespace 
#endif // BIT_INT_ITR_HPP_
