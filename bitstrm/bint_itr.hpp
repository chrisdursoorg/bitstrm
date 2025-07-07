// bint_itr.hpp
//
// For sub-bitstrms that store arrays of fixed sized integers the bint_itr (as
// either mutable_bint_itr and const_bint_itr) allow for standard access to the
// underlying direct and contiguous signed and unsigned (reg/ureg) integer arrays

#include <iostream>
#include <iterator>
#include <bitstrm/bref.hpp>
#include <cassert>
#include <utility>

namespace bitstrm{

  struct bint_itr_base {

    using iterator_category = std::random_access_iterator_tag;
    using size_type         = ureg;
    using difference_type   = std::ptrdiff_t;
    
    bint_itr_base(){}
    bint_itr_base(bref p, unsigned bz): m_ptr(p), m_bsize(bz){
      assert(bz <= c_register_bits);
    } 
    bint_itr_base(bint_itr_base const& rhs){ assign(rhs); }

    bint_itr_base& operator=(bint_itr_base const& rhs){
      assign(rhs);
      return *this;
    }

    inline bool operator==(bint_itr_base const& rhs)const{
      return m_ptr == rhs.m_ptr;
    }

    inline bool operator!=(bint_itr_base const& rhs)const{
      return m_ptr != rhs.m_ptr;
    }

    inline bool operator<(bint_itr_base const& rhs)const{
      return m_ptr < rhs.m_ptr;
    }

    inline bool operator>(bint_itr_base const& rhs)const{
      return m_ptr > rhs.m_ptr;
    }

    inline bool operator<=(bint_itr_base const& rhs)const{
      return m_ptr <= rhs.m_ptr;
    }

    inline bool operator>=(bint_itr_base const& rhs)const{
      return m_ptr >= rhs.m_ptr;
    }

    inline difference_type operator-(bint_itr_base const & rhs){
      assert(m_bsize == rhs.m_bsize && "comparing iterators of different bsize");
      return (m_ptr-rhs.m_ptr)/m_bsize;
    }
    
  protected:

    inline bint_itr_base& assign(bint_itr_base const& r){
      m_ptr   = r.m_ptr;
      m_bsize = r.m_bsize;
      return *this;
    }

    inline void adv(difference_type d){ m_ptr += d*m_bsize; }

    bref     m_ptr;
    unsigned m_bsize;
  };  // class bint_itr_base


  // bint_proxy
  //
  // facilitates the mutable part of mutable_bint_itr by holding a reference to
  // the underlying bits implementing the read/write operations 
  template<class SU_>
  struct bint_proxy {

    bint_proxy();
    bint_proxy(bref const& b, unsigned bsz):  m_ptr(b),  m_bsize(bsz){}
    bint_proxy( bint_proxy const&) = default;
    
    operator SU_(){ return read();}
    
    bint_proxy& operator=(bint_proxy const& rhs){write(rhs.read()); return *this;}
    bint_proxy& operator=(SU_ v){write(v); return *this;}

    SU_  read()const       { return m_ptr.read<SU_>(m_bsize);}
    void write(SU_ v)const { m_ptr.write(v, m_bsize);}

    bool operator==(bint_proxy const& __x) const { return read() == __x.read(); }
    bool operator< (bint_proxy const& __x) const { return read() <  __x.read(); }

    friend void swap(bint_proxy const& lhs, bint_proxy const& rhs)noexcept{
      SU_ tmp = lhs.read();
      lhs.write(rhs.read());
      rhs.write(tmp);
    }

    friend void swap(bint_proxy& lhs, bint_proxy& rhs)noexcept{
      swap(const_cast<bint_proxy const&>(lhs),
	   const_cast<bint_proxy const&>(rhs));
    }

    friend void swap(bint_proxy const& lhs, SU_& rhs)noexcept{
      SU_ tmp = lhs.read();
      lhs.write(rhs);
      rhs = tmp;
    }

    friend void swap(SU_& lhs, bint_proxy const& rhs)noexcept{
      SU_ tmp = lhs;
      lhs = rhs.read();
      rhs.write(tmp);
    }

    bref     m_ptr;
    unsigned m_bsize;
  };

  // mutable_bint_itr
  //
  // non-const or mutable iterator for an array of Signed or Unsigned integers of
  // fixed bsize.  Mutability brings with it the same advantages and disadvantages
  // of proxy (bint_proxy) or non-direct access with standard algorithms.  It
  // should mostly work with all standard algorithms but might present problems
  // e.g. the address of reference *is not* pointing to the respective value_type.
  
  template<class SU_>
  struct mutable_bint_itr : public bint_itr_base {

    using my_t       = mutable_bint_itr<SU_>;
    using value_type = SU_;
    using reference  = bint_proxy<SU_>;

    mutable_bint_itr() = default;
    mutable_bint_itr(mutable_bint_itr const&) = default;
    mutable_bint_itr(bref p, unsigned bsz): bint_itr_base(p, bsz){}
    
    my_t& operator+=(const difference_type& d){ adv(d); return *this;}
    my_t& operator-=(const difference_type& d){ adv(-d);return *this;}
    my_t& operator++(){adv(1); return *this;}
    my_t& operator--(){adv(-1);return *this;}
    my_t  operator++(int){auto t(*this);adv(1);return t;}
    my_t  operator--(int){auto t(*this);adv(-1);return t;}
    my_t  operator+(difference_type d)const {auto t(*this); t.adv(d); return t;}
    my_t  operator-(difference_type d)const{auto t(*this); t.adv(-d); return t;}
    difference_type operator-(my_t const& d){return bint_itr_base::operator-(d);}

    // reference accessors
    reference operator*()const{ return reference {m_ptr, m_bsize}; }
    reference operator[](difference_type i) const { return *(*this + i); }
  };

  // const_bint_itr
  //
  // Const variant of bint_itr is somewhat better behaved than mutable_bint_itr
  // as it does not have the baggage of a proxy object. Instead it dereferences to
  // a value (a S/U integer) and not an underlying object.  This potentially may
  // still fail with standard algorithms that rely on the address of the
  // dereferenced object.
  //
  // This class should preferentially be used over mutable_bint_itr in read-only
  // operations to reduce potential conflict and improve performance.
 
  template<class SU_>
  struct const_bint_itr : public bint_itr_base {

    using my_t       = const_bint_itr<SU_>;
    using value_type = SU_;
    using reference  = SU_;

    const_bint_itr() = default;
    const_bint_itr(const_bint_itr const& o) : bint_itr_base(o){}
    const_bint_itr(mutable_bint_itr<SU_> const& r): bint_itr_base(r) {}
    const_bint_itr(bref p, unsigned bsz): bint_itr_base(p, bsz){}
    
    my_t& operator+=(const difference_type& d){ adv(d); return *this;}
    my_t& operator-=(const difference_type& d){ adv(-d);return *this;}
    my_t& operator++(){adv(1); return *this;}
    my_t& operator--(){adv(-1);return *this;}
    my_t  operator++(int){auto t(*this);adv(1);return t;}
    my_t  operator--(int){auto t(*this);adv(-1);return t;}
    my_t  operator+(difference_type d)const{auto t(*this); t.adv(d); return t;}
    my_t  operator-(difference_type d)const{auto t(*this); t.adv(-d); return t;}
    difference_type operator-(my_t const& d){return bint_itr_base::operator-(d);}
    
    my_t& operator=(mutable_bint_itr<SU_> const& rhs){ assign(rhs); return *this;}
    my_t& operator=(my_t const& rhs){assign(rhs); return *this;}

    reference operator*()const{ return m_ptr.read<SU_>(m_bsize); }
    reference operator[](difference_type i) const { return *(*this + i); }
  };
}
