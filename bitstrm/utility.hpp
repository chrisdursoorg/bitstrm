// utility.hpp
//
// functions and utilities going great lengths to avoid branching for
// inclusion in inner loops, these functions also deal with the
// packing and unpacking of signed or unsigned values to and from reg
// / ureg types

#ifndef BITSTRM_UTILITYHPP_
#define BITSTRM_UTILITYHPP_


#include <limits.h>
#include <limits>
#include <type_traits>
#include <utility>
#include <algorithm>
#include <iostream>
#include <cassert>

#include "bitstrm/reg.hpp"


namespace  bitstrm { 

  // numeric_limits_signed
  // given bsize:{_min, _max} = {-(2^(n-1), 2^(n-1)-1 } 
  inline constexpr reg  numeric_limits_signed_min  (unsigned bsize) {
    return bsize ? -( reg(1) << (bsize-1))        : 0;
  } 
  inline constexpr reg  numeric_limits_signed_max  (unsigned bsize) {
    return bsize ?  ((reg(1) << (bsize-1)) - 1 )  : 0;
  } 
  inline constexpr ureg numeric_limits_unsigned_min(unsigned ) {
    return  0;
  } 
  inline constexpr ureg numeric_limits_unsigned_max(unsigned bsize) {
    return bsize ?  ((ureg)((ureg(1) << (bsize - 1)) << 1) -1)  : 0;
  } 

  // abs
  //
  // fast absolute value adapted from
  // 'http://graphics.stanford.edu/~seander/bithacks.html#IntegerAbs'
  // fails with (as does std::abs) for MIN_INT due to overflow
  template<typename _T>
  inline constexpr typename std::make_unsigned<_T>::type abs(_T v) {
    _T const mask = v >> ((sizeof(_T)*CHAR_BIT)-1); return (v+mask)^mask;
  } 

  // bit_sign_adj
  //
  // min_bits for a container of unsigned integers can be determined
  // with bit-or accumulation and then min_bits of the resulting value
  // with bit_sign_adj this same trick can be played with signed
  // integers by turning signed positive/negative numbers into same
  // bit size analogs note that you have to return 0 for input zero.
  // The function is identity with unsigned type.
  //
  // USAGE acc = 0; for_each(b,e, [](signed_or_unsigned_type v){ acc
  //  |= bit_sign_adj(v); }); min = min_bits(acc);
  // 
  //
  // if IS_SIGNED type is true then shift left any (non zero) value
  // for signed bit space Always valid, e.g. {max,min} signed int will
  // result in word number of bits, 0->0, etc.
  template<typename _T>
  inline constexpr ureg bit_sign_adj(_T v, std::false_type)  { return v;}

  template<typename _T>
  inline constexpr ureg bit_sign_adj(_T v, std::true_type) { 
    // LOGIC TABLE, note we only care about msb
    //  GIVEN        RETURN
    //   v >= 0   => v << 1;
    //   v == -1  => 1;
    //  (v < -1)  => (~v) << 1 | 1;
    // this '| 1' is a benign side effect of
    // raising negative numbers to odd value
    ureg isNeg  = ((ureg) v) >> (c_register_bits-1);  // [0] -> 0, [2,3] -> 1 
    ureg negMsk = ((reg)  v) >> (c_register_bits-1);  // [0] -> 0,
                                                      // [2,3] -> 0xFF..F
    
    return (((((ureg)~v) & negMsk) | (((ureg)v) & ~negMsk))  << 1) | isNeg;
  }
    
  template<typename _T> inline constexpr ureg bit_sign_adj(_T v) {
    return bit_sign_adj(v, std::is_signed<_T>());
  }

  // min_bits
  //
  // 2s Compliment Representation of integers of arbitrary length
  //
  // returns:
  //
  // 0 when val == 0
  // 
  // The minimum number of bits for unsigned type are those that are
  // necessary to retain in order to store val
  //
  // The minimum number of bits for signed type are those that are
  // necessary to retain in order to maintain val including one
  // leading signed bit
  //
  // v: 0 -> 0, else v: floor(ln2(v))
  // signed variants, add one signed bit (if not 0)
  // _ITR a != b, otherwise result would not be defined!
  template<typename _T>
  constexpr ureg min_bits(_T val);

////////////////////////////////////////////////////////////////////////////////        
  template<>
  inline constexpr ureg
  min_bits<unsigned int>      (unsigned int v)      {
    return v ? static_cast<ureg>( (sizeof(unsigned)*CHAR_BIT)
                                  - __builtin_clz (v))   : 0;
  }
  template<>
  inline constexpr ureg
  min_bits<unsigned long >    (unsigned long v)     {
    return v ? static_cast<ureg>( (sizeof(unsigned long)*CHAR_BIT)
                                  - __builtin_clzl (v))  : 0;
  }
  template<>
  inline constexpr ureg
  min_bits<unsigned long long>(unsigned long long v){
    return v ? static_cast<ureg>( (sizeof(unsigned long long)*CHAR_BIT)
                                  - __builtin_clzll(v))  : 0;
  }
  template<>
  inline constexpr ureg
  min_bits<signed int>        (signed int v)        {
    return v ? static_cast<ureg>( (sizeof(signed int)*CHAR_BIT)
                                  - __builtin_clz  (bit_sign_adj(v))) : 0;
  }
  template<>
  inline constexpr ureg
  min_bits<signed long >      (signed long v)       {
    return v ? static_cast<ureg>( (sizeof(signed  long)*CHAR_BIT)
                                  - __builtin_clzl (bit_sign_adj(v))) : 0;
  }
  template<>
  inline constexpr ureg
  min_bits<signed long long>  (signed long long v)  {
    return v ? static_cast<ureg>( (sizeof(signed  long long)*CHAR_BIT)
                                  - __builtin_clzll(bit_sign_adj(v))) : 0;
  }
  
  template<typename _ITR>
  inline constexpr ureg min_bits(_ITR b, _ITR e){
    ureg v(0); 
    
    while( !(b == e)) 
      v |= bit_sign_adj(*b++);
      
    return min_bits(v);
  }

  // signextend
  //
  // C/C++ black magic bit field behavior extends sign @ BIT_COUNT to T
  //
  // http://graphics.stanford.edu/~seander/bithacks.html#FixedSignExtend
  // Signed extention from BIT_COUNT bit width to bit width of type T
  //
  // {true_type, false_type} variants {handle, no-op is_signed(x)}
  // respectively
  template <typename T, unsigned BIT_COUNT>
  inline constexpr T
  signextend(const T x) {
    // sba should_be_anonymous
    struct sba{sba(){}; T x:BIT_COUNT;} s;
    return s.x = x;
  }
    
  template<typename T, unsigned BIT_COUNT>
  inline constexpr T
  signextend(const T x, std::true_type){
    return signextend<T, BIT_COUNT>(x);
  }
  
  template<typename T, unsigned BIT_COUNT>
    inline constexpr T signextend (const T x, std::false_type){
    return x;
  }

  // clz
  //
  // Count leading zero's.  Zero returns the number of bits in the register.

  template<typename BASE>
  inline BASE op_clz(BASE);

  template<>
  inline uint64_t op_clz<uint64_t>(uint64_t i ){ return i ? __builtin_clzll(i): 64; }

  template<>
  inline uint32_t op_clz<uint32_t>(uint32_t i ){ return i ? __builtin_clz(i): 32; }

  template<class BASE_> unsigned op_pop_count(BASE_);

  template<> inline unsigned op_pop_count<unsigned>(unsigned x)                    { return __builtin_popcount(x);   }
  template<> inline unsigned op_pop_count<unsigned long>(unsigned long x)          { return __builtin_popcountl(x);  }
  template<> inline unsigned op_pop_count<unsigned long long>(unsigned long long x){ return __builtin_popcountll(x); }

  
}

#endif // def'd BITLIB_UTILITYHPP_
