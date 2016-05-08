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

#include <iterator>
#include <iostream>


 namespace bitint {

    /// Cx02/98 approach in addition to swap need to do other std::algos, the pattern is 
    /// vector<bool>::reference, truly standards uglyness
    
    template<int _BITSZ, typename SIGN_UNSIGN>
    struct _bit_int_reference
    {
      
      _bit_int_reference(bitstrm p): p_(p){}
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
    
      inline SIGN_UNSIGN read()const           { return p_.read_as<SIGN_UNSIGN>(_BITSZ); }
      inline void        write(SIGN_UNSIGN __x){        p_.write(_BITSZ, __x); } 

    }; // struct _bit_int_reference


    // swap
    //
    // swaps deep down to the undrlying bit-integer
    template<int _B, typename _S>
    void swap(_bit_int_reference<_B,_S> lhs, _bit_int_reference<_B,_S> rhs){
      //    std::cout << "PRE SWAP{ lhs: "; lhs.print(std::cout); std::cout << " rhs: "; rhs.print(std::cout); std::cout << " }" << std::endl; 
      _S t = lhs;
      lhs = (_S)rhs;
      rhs = t;
      //    std::cout << "POST SWAP{ lhs: "; lhs.print(std::cout); std::cout << " rhs: "; rhs.print(std::cout); std::cout << " }" << std::endl; 
    }

    template<int _B, typename _S>
    void swap(_S& lhs, _bit_int_reference<_B,_S> rhs){
      _S tmp = lhs;
      lhs = rhs;
      rhs = tmp;
    }

    template<int _B, typename _S>
    void swap(_bit_int_reference<_B,_S> lhs, _S& rhs){
      _S tmp = lhs;
      lhs = rhs;
      rhs = tmp;
    }

    // 98/03 approach bit_int_itr
    // consider also adding 
    //   swap
    //   swap_range
    //   rotate
    //   equal
    // to _bit_int_reference to follow the std::vector<bool> specialization as an example


    template<int _BITSZ, typename SIGN_UNSIGN>
    struct bit_int_itr : public std::iterator<std::random_access_iterator_tag, SIGN_UNSIGN>{
      bitstrm p_;

      bit_int_itr(bitstrm p): p_(p) {}
    
      typedef _bit_int_reference<_BITSZ, SIGN_UNSIGN> reference;
      typedef bit_int_itr<_BITSZ, SIGN_UNSIGN> iterator;
    
      reference operator*()                       { return reference(this->p_); }

      iterator& operator++()                      { this->p_ += _BITSZ; return *this; }    
      iterator  operator++(int)                   { iterator t = *this; this->p_ += _BITSZ; return t; }
      iterator& operator--()                      { this->p_ -= _BITSZ; return *this; }    
      iterator  operator--(int)                   { iterator t = *this; this->p_ -= _BITSZ; return t; }
      iterator& operator+=(std::ptrdiff_t n)      { this->p_ += _BITSZ*n; return *this; }    
      iterator& operator-=(std::ptrdiff_t n)      { this->p_ -= _BITSZ*n; return *this; }    
      iterator  operator+ (std::ptrdiff_t n)const { iterator t = *this; return t += n; }
      iterator  operator- (std::ptrdiff_t n)const { iterator t = *this; return t -= n; }
      reference operator[](std::ptrdiff_t n)const { return *(*this + n); }


      bool operator==(const bit_int_itr& rhs) const { return p_ == rhs.p_; }  
      bool operator< (const bit_int_itr& rhs) const { return p_ < rhs.p_;  }
      bool operator!=(const bit_int_itr& rhs) const { return p_ != rhs.p_; }
      bool operator> (const bit_int_itr& rhs) const { return p_ > rhs.p_;  }
      bool operator<=(const bit_int_itr& rhs) const { return p_ <= rhs.p_; }
      bool operator>=(const bit_int_itr& rhs) const { return p_ >= rhs.p_; }
  
    }; // struct bit_int_itr

    template<int _B, typename _S>
    inline std::ptrdiff_t operator-(const bit_int_itr<_B,_S>& lhs, const bit_int_itr<_B,_S>& rhs){ return bitstrm::subtract(lhs.p_, rhs.p_)/_B; }

 }  // namespace bitint
#endif // BIT_INT_ITR_HPP_
