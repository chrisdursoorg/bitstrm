// reg.hpp

#ifndef REG_HPP_
#define REG_HPP_


// The bitstrm library is optimized on reading writing a subset of a
// reg(ister) of bits and recalling that value in a reg(ister) of
// either signed or unsigned ureg type.  Stitching will occur at the
// register boundary, hence a larger register choice (all other things
// being the same) would be quicker than a shorter one.
//
// Regarding sub register addressing, experimentation on x86
// architecture resulted in endian adjustment with no loss in
// performance.  Full implementon (__builtin_bswap) might prove useful
// in applications packing sub register sized fields.  Currently
// implementation and testing for this feature is not warranted.

#include <type_traits>
#include <cstdint>
#include <bit>

namespace bitstrm { 
 
  // reg
  // 
  // base memory primitive, should be largest available to minimize
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
  // return mask (right aligned) of bsize bits,
  // mask  defined for bsize : (0, c_register_bits]
  // mask0 defined for bsize : [0, c_register_bits] 
  inline reg mask(unsigned bsize) {return ureg(-1)>>(c_register_bits-bsize); }
  inline reg mask0(unsigned bsize){return bsize ? mask(bsize) : 0; }
  
}

#endif // REG_HPP_
