// utility.hpp
//
// functions and utilities going great lengths to avoid branching for inclusion in inner loops, these functions also deal with the packing and unpacking
// of signed or unsigned values to and from reg / ureg types

#ifndef BITSTRM_UTILITYHPP_
#define BITSTRM_UTILITYHPP_


#include <limits.h>
#include <limits>
#include <type_traits>
#include <boost/type_traits/is_signed.hpp>
#include <boost/type_traits/is_integral.hpp>
#include <utility>
#include <algorithm>
#include <iostream>
#include <ctime>


#include "bitstrm/reg.hpp"


namespace bitint {


#if 0  
    // the following is kept here for reference only, as the testing with pentium appears to operate as though LT operation is equivalent to branch
    
    // max/min todo for primative types use branch free version  http://graphics.stanford.edu/~seander/bithacks.html
    // foreach integer types, else revert to std:: implementation 
    //
    // Chose not to do this with templates because, bit approach returns value not reference.  Could still do this with specialization of return type,
    // but plain ol macros seep pretty clean approach
    //
    // NOTE: INT_MIN <= x - y <= INT_MAX, for non std approaches
    // 

    
#define BITLIB_MINMAX_IMPL(TYPE)					 \
    inline TYPE min(TYPE x, TYPE y){ return y ^ ((x ^ y) & -(x < y)); }; \
    inline TYPE max(TYPE x, TYPE y){ return x ^ ((x ^ y) & -(x < y)); }; \
    inline std::pair<TYPE,TYPE> minmax(TYPE x, TYPE y) {  TYPE i =  ((x ^ y) & -(x < y)); return std::make_pair(y^i, x^i); }


 // THIS WORKS ONLY WITH SIGNED, AND THEN PERFORMANCE IS NOT WORKING AS EXPECTED!
#define BITLIB_MINMAX_IMPL(TYPE)   \
    inline TYPE min( TYPE x, TYPE y) { return y + ((x - y) & (((x - y) >> (sizeof(TYPE) * CHAR_BIT - 1)))); } \
    inline TYPE max( TYPE x, TYPE y) { return x - ((x - y) & (((x - y) >> (sizeof(TYPE) * CHAR_BIT - 1)))); } \
    inline std::pair<TYPE,TYPE> minmax(TYPE x, TYPE y) {  TYPE i =  ((x - y) & ((x - y) >> (sizeof(TYPE) * CHAR_BIT - 1))); return std::make_pair(y+i, x-i); }
    
    BITLIB_MINMAX_IMPL(bool)
    BITLIB_MINMAX_IMPL(char)
    BITLIB_MINMAX_IMPL(unsigned char)
    BITLIB_MINMAX_IMPL(short)
    BITLIB_MINMAX_IMPL(unsigned short)
    BITLIB_MINMAX_IMPL(int)
    BITLIB_MINMAX_IMPL(unsigned int)
    BITLIB_MINMAX_IMPL(long)
    BITLIB_MINMAX_IMPL(unsigned long)
    BITLIB_MINMAX_IMPL(long long)
    BITLIB_MINMAX_IMPL(unsigned long long)

#endif 
    
    // Optomism I guess, perhaps I will develop or discover a better min/max/minmax or perhaps the pentium die will change
    // but for now std implementation 
    
    template<typename ANYTYPE> 
    inline const ANYTYPE& max(const ANYTYPE& a, const ANYTYPE& b){ return std::max(a,b); }
    template<typename ANYTYPE> 
    inline const ANYTYPE& min(const ANYTYPE& a, const ANYTYPE& b){ return std::min(a,b); }
    template<typename ANYTYPE> 
    inline std::pair<const ANYTYPE&, const ANYTYPE&> minmax(const ANYTYPE& a, const ANYTYPE& b){ return std::minmax(a,b); }
  

    // abs
    //
    // fast absolute value adapted from 'http://graphics.stanford.edu/~seander/bithacks.html#IntegerAbs'
    // fails (as does std::abs) for MIN_INT due to overflow
    template<typename _T>
    inline typename std::make_unsigned<_T>::type abs(_T v) { _T const mask = v >> ((sizeof(_T)*CHAR_BIT)-1); return (v+mask)^mask; }; 

    // bit_sign_adj
    //
    // min_bits for a container of unsigned integers can be determined with bit-or accumulation and then min_bits of the resulting value
    // with bit_sign_adj this same trick can be played with signed integers by turning signed positive/negative numbers into same bit size analogs
    // note that you have to return 0 for input zero, and the function becomes Identity automatically when unsigned type
    //
    // USAGE  acc = 0; for_each(b,e, [](signed_or_unsigned_type v){ acc |= bit_sign_adj(v); }); min = min_bits(acc);
    // 
    //
    // if IS_SIGNED type is true then shift left any (non zero) value for signed bit space
    // Always valid, e.g. {max,min} signed int will result in word number of bits, 0->0, etc.
    template<typename _T> inline ureg bit_sign_adj(_T v, boost::false_type)  { return v;}

    template<typename _T> inline ureg bit_sign_adj(_T v, boost::true_type /*meansIsReallySigned*/) 
    { 
      // LOGIC TABLE, note we only care about msb
      //  GIVEN        RETURN
      //   v >= 0   => v << 1;
      //   v == -1  => 1;
      //  (v < -1)  => (~v) << 1 | 1;  // this '| 1' is a benign side effect "raise negative numbers to odd value"
      
      ureg isNeg  = ((ureg) v) >> (c_register_bits-1);  // [0] -> 0, [2,3] -> 1 
      ureg negMsk = ((reg)  v) >> (c_register_bits-1);  // [0] -> 0, [2,3] -> 0xFF..F
      
      return (((((ureg)~v) & negMsk) | (((ureg)v) & ~negMsk))  << 1) | isNeg;
    }
    
    template<typename _T> inline ureg bit_sign_adj(_T v) { return bit_sign_adj(v, boost::is_signed<_T>()); }

    // min bits
    //
    // The minimum number of bits for unsigned type are those that are necessrary to retain in order to maintain val
    //
    // The minimum number of bits for signed type are those that are necessary to retain in order to maintain val, when
    // carrying  (via signextend) the min_bits(val)
    //
    // v: 0 -> 0, else v: floor(ln2(v))
    // signed variants, add one signed bit (if not 0)
    // _ITR a != b, otherwise result would not be defined!
    template<typename _T>  ureg min_bits(_T val);
    template<> inline ureg min_bits<unsigned int>      (unsigned int v)      {return v ? static_cast<ureg>( (sizeof(unsigned)*CHAR_BIT)           - __builtin_clz (v))   : 0;}
    template<> inline ureg min_bits<unsigned long >    (unsigned long v)     {return v ? static_cast<ureg>( (sizeof(unsigned long)*CHAR_BIT)      - __builtin_clzl (v))  : 0;}
    template<> inline ureg min_bits<unsigned long long>(unsigned long long v){return v ? static_cast<ureg>( (sizeof(unsigned long long)*CHAR_BIT) - __builtin_clzll(v)) : 0;}
    template<> inline ureg min_bits<signed int>        (signed int v)        {return v ? static_cast<ureg>( (sizeof(signed int)*CHAR_BIT)         - __builtin_clz  (bit_sign_adj(v))) : 0;}
    template<> inline ureg min_bits<signed long >      (signed long v)       {return v ? static_cast<ureg>( (sizeof(signed  long)*CHAR_BIT)       - __builtin_clzl (bit_sign_adj(v))) : 0;}
    template<> inline ureg min_bits<signed long long>  (signed long long v)  {return v ? static_cast<ureg>( (sizeof(signed  long long)*CHAR_BIT)  - __builtin_clzll(bit_sign_adj(v))) : 0;}
    
    template<typename _ITR> inline unsigned min_bits(_ITR b, _ITR e)   { 
      ureg v(0); 
      
      while( !(b == e)) 
	v |= bit_sign_adj(*b++);
      
      return min_bits(v);
    }  

  
    // signextend
    //
    // http://graphics.stanford.edu/~seander/bithacks.html#FixedSignExtend
    // Signed extention from BIT_COUNT bit width to bit width of type T
    //
    // {true_type, false_type} variants {handle, no-op is_signed(x)} respectively
    template <typename T, unsigned BIT_COUNT>
    inline T signextend(const T x)
    {
      struct {T x:BIT_COUNT;} s;
      return s.x = x;
    }
    
    template<typename T, unsigned BIT_COUNT>
    inline T signextend (const T x, boost::true_type){ return signextend<T, BIT_COUNT>(x); }
  
    template<typename T, unsigned BIT_COUNT>
    inline T signextend (const T x, boost::false_type){ return x; }

    
    struct TimeFixture
    {
      TimeFixture(const char* name, size_t unit = 0) 
	: m_start(std::chrono::system_clock::now())
	, m_name(name)
	, m_number(s_timer_number++)
	, m_unit(unit) 
    
      {
	std::cout << "TIMER START [" << m_number << "] - " << m_name << std::endl;
      }
 
      ~TimeFixture()
      {
	std::chrono::time_point<std::chrono::system_clock> end = std::chrono::system_clock::now();
	std::chrono::duration<double> elapsed_seconds = end - m_start;
	std::time_t end_time = std::chrono::system_clock::to_time_t(end);

	char mbstr[100];
	std::strftime(mbstr, sizeof(mbstr), "%X", std::localtime(&end_time));
	std::cout << "TIMER END [" << m_number << "] " << m_name << " -  completed @ " << mbstr << " elapsed time: " << elapsed_seconds.count() << "s";
	if(m_unit) {
	  double per_unit = elapsed_seconds.count() / m_unit;
	  std::cout << " @" << per_unit << " s/cycle freq: " << 1./per_unit << "/s";
	}
      
	std::cout << std::endl;
      }

      static unsigned s_timer_number;
      std::chrono::time_point<std::chrono::system_clock> m_start;
      const char* m_name;
      const unsigned m_number;
      size_t m_unit;
    };

}



#endif // def'd BITLIB_UTILITYHPP_
