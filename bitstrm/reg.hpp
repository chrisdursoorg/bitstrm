// reg.hpp

#ifndef REG_HPP_
#define REG_HPP_

#ifdef NO_ENDIAN_ADJUSTMENT
#else
#define USE_ENDIAN
#endif

// This bit integer library if for reading byte streams as arrays of arbitrary length (as in bit) signed or unsigned integers.
// That being said types reg::reg and reg::ureg hold that arbitary sized value as either an signed or unsigned respectively
// 'register' values


#include <type_traits>
#include <stdint.h>


namespace bitint {
 
    // reg
    // 
    // base memory primative, should be largest available to minimize split register occurances unless is otherwise sub-optimum 
    ////////////////////////////
    // 
    typedef int64_t  reg;    
    //
    ////////////////////////////

    // intermediate for applying bit manipulation
    typedef std::make_unsigned<reg>::type ureg;   

    const unsigned c_register_bits          = sizeof(reg)*__CHAR_BIT__;          // how many bits in a reg(ister)

    template<unsigned REG_BITS>
    struct address_bit_size{};

    template<> struct address_bit_size<16>  { static const unsigned value = 4; };
    template<> struct address_bit_size<32>  { static const unsigned value = 5; };
    template<> struct address_bit_size<64>  { static const unsigned value = 6; };
    template<> struct address_bit_size<128> { static const unsigned value = 7; };


    const unsigned c_register_bit_addr_sz   = address_bit_size<c_register_bits>::value;   // how many bits to sub address reg/ureg
    const unsigned c_register_bit_addr_msk = ( 1 << c_register_bit_addr_sz) -1;           // the bit mask of the above


    // PERFORMANCE NOTES
    // even though USE_ENDIAN results in a significant number of operations timing on 'Intel Core i7' is approximately just as
    // quickly with and without the bswap
#ifdef USE_ENDIAN
    // this variant allows for ix86 to correctly address sub-register byte memory, prior experiments indicated
    // that this bswap added negligable costs
    // PLATFORM DEPENDENCE
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
  
}



#endif // REG_HPP_
