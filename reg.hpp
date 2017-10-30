// reg.hpp

#ifndef REG_HPP_
#define REG_HPP_

#ifdef NO_ENDIAN_ADJUSTMENT
#else
#define USE_ENDIAN
#endif

// The bitstrm library is optimized on reading writing a small number
// of bits and recalling that value in a reg(ister) (of signed or
// unsigned ureg type).  Stiching will occur at the register boundry,
// hence a larger register choice (all other things being the same)
// would be quicker than a shorter one, plus the library resticts the
// maximum number of bits of bsize in an integer to the register size.
//
// The only other definitions in this file are for ENDIAN adjustment.
// This would be relavent only when the buffer holding your bits is
// byte allocated and not reg allocated, for instance if you held your
// bits in byte arrays on an x86 (e.g. string containers) then you
// would turn off endian_adjustment at your peril.  I have never
// witness performance degredation from builtin_bswap.


#include <type_traits>
#include <stdint.h>

namespace bitstrm { 
 
  // reg
  // 
  // base memory primative, should be largest available to minimize
  // register crossing
  
  typedef int64_t  reg;    

  // intermediate for applying bit manipulation
  typedef std::make_unsigned<reg>::type ureg;   

  // how many bits in a reg(ister)
  const unsigned c_register_bits          = sizeof(reg)*__CHAR_BIT__;

  template<unsigned REG_BITS>
  struct bits_2{};
    
  // NOTE that the address_bsize does NOT reach maximal value span of
  // the register! Zero bits being a very frequent bsize
  // a) reduces the span to k-1 bits or
  // b) results in you needing another bit for
  // your address and that bit is hardly ever if ever used.
  
  template<> struct bits_2<16>  {
    static const unsigned address_bsize = 4;
    typedef uint16_t    unsigned_type;
  };
  template<> struct bits_2<32>  {
    static const unsigned address_bsize = 5;
    typedef uint32_t    unsigned_type;
  };
  template<> struct bits_2<64>  {
    static const unsigned address_bsize = 6;
    typedef uint64_t    unsigned_type;
  };
  template<> struct bits_2<128> {
    static const unsigned address_bsize = 7;
    typedef __uint128_t unsigned_type;
  };

  // how many bits to sub address reg/ureg
  const unsigned c_register_bit_addr_sz   = bits_2<c_register_bits>::address_bsize;
  // the bit mask of the above
  const unsigned c_register_bit_addr_msk = ( 1 << c_register_bit_addr_sz) - 1;           


  // PERFORMANCE NOTES even though USE_ENDIAN results in a significant
  // number of operations timing on 'Intel Core i7' is approximately
  // just as quickly with and without the bswap
#ifdef USE_ENDIAN
  inline int64_t endian_adj(const int64_t& r) {return __builtin_bswap64(r); }
#else
  // no-op simply read/write memory as per native bit outlay
  inline int64_t endian_adj(const  int64_t& r) {return r; }
#endif

  // reg_ureg
  // define type reg if is_signed<IS_SIGNED> else ureg
  template<typename IS_SIGNED> struct reg_ureg            { typedef ureg value_type; };
  template<>                   struct reg_ureg <char>     { typedef  reg value_type; };
  template<>                   struct reg_ureg <short>    { typedef  reg value_type; };
  template<>                   struct reg_ureg <int>      { typedef  reg value_type; };
  template<>                   struct reg_ureg <long>     { typedef  reg value_type; };
  template<>                   struct reg_ureg <long long>{ typedef  reg value_type; };


  // mask
  //
  // return mask (right alligned) of bsize bits,
  // mask  defined for bsize : (0, c_register_bits]
  // mask0 defined for bsize : [0, c_register_bits] (placeholder for branch free)
  inline ureg mask(unsigned bsize) {return ureg(-1)>>(c_register_bits-bsize); }
  inline ureg mask0(unsigned bsize){return bsize ? mask(bsize) : 0; }
  
}

#endif // REG_HPP_
