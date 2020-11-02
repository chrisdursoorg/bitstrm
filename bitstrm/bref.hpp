// bitstrm/bref.hpp
//
#ifndef BITSTRM_BREF_HPP
#define BITSTRM_BREF_HPP

#include <cassert>
#include "bitstrm/utility.hpp"


// bref(erence)
//
// Refers to a location in an allocated bitstrm assuming external
// type/bsize management.
//
// The 'pointer' or 'iterator' like behavior assumes a single bit
// type, hence r1 - r0 is the bitwise distance or bsize, *r0 is the
// bit value at r0, r0++ avavances the reference by 1 bit, etc.
//
// Two bref r0, r1 can consitute a range [r0, r1) if constructed from
// the same base address over contiguous reg castable memory.
//
// Milti bit access allows for the mapping of [0,
// c_register_bit_addr_sz) bits to/from signed (reg) or unsigned (ureg)
// values using various coding and meta information
//

////////////////////////////////////////////////////////////////////////////////
//                   ENCODINGS
//
//  write  value with signed or unsigned values as it is masked off after bsize
//         bits, consequentially
//              * only one interface is required
//              * at least 1 2's compliment leading bit must be encoded for
//                signed storage (e.g. correctly typed min_bits(value))
//              * write must be matched with the intended read<REG_UREG>
//         e.g. {1, 1, 01, 01, 101, 101} -> { -1, 1, +1, 1, -3, 5}
//
//         buf.write(0b110101101101, 12);
//         assert(buf.iread<reg> (1) == -1);
//         assert(buf.iread<ureg>(1) ==  1U);
//         assert(buf.iread<reg> (2) ==  1);
//         assert(buf.iread<ureg>(2) ==  1U);
//         assert(buf.iread<reg> (3) == -3);
//         assert(buf.iread<ureg>(3) ==  5U);
//
//  clz    count leading zeros
//         e.g. {1, 01, 001, 0001} <-> {0, 1, 2, 3}
//         buf.write(1, zeros + 1);       // write zeros followed by a one
//         restore = buf.iclz);
//         assert(zeros == restore);
//
//  rls    run length (externally) specified value, given a bsize infer the
//         magnitude (msb) encoding the mantissa optimally
//         e.g. {'', '0', '00', '11'} -> {0, 1, 3, 6}
//         DISTINCT FROM WRITE write_rls MUST BE correctly signed AND matched
//         e.g. {'', '0', '00', '11'} -> {+0, +1, +2, -4}
//         e.g. {'1', '10', '101', '1110'} -> {-1, -3, -6, -9}
//         
//         beg = buf;
//         
//  rle    packet run length encode, (next prefix) kbit packet size, kbit > 1
//         (e.g. k:5 {0|0000, 1|1100^0|0001} -> {0, 193})
//
//  rles   packetsigned integer run length encode, (next prefix) kbit packet size
//         (e.g. k:5 {0|0000, 1|1100^0|0001} -> {0, -63})
//

// run length specified maps integers with specified or listed
// as in [prefix]{(2^(*prefix)-1)} bits, values of 
// [0, 2^(total_bits-prefix) -2]

namespace bitstrm {

  struct bref {
    
  private:
    reg* m_addr;
    reg  m_off;
    
  public:
    bref(){};
    bref(const bref& rhs): m_addr(rhs.m_addr), m_off(rhs.m_off){}
    bref(void* addr): m_addr(reinterpret_cast<reg*>(addr)), m_off(0){}
    bref(void* addr, int off): m_addr(reinterpret_cast<reg*>(addr)),
                               m_off(off){ norm();}
    constexpr bref(bref&& rhs) = default;
    
    // STANDARD OPERATORS
    ureg  operator*()const;
    bref& operator= (const bref& rhs) = default;
    bref& operator= (bref&& rhs) = default;
    bool  operator==(bref const& rhs)const;
    bool  operator<=(bref const& rhs)const;
    bool  operator>=(bref const& rhs)const;
    bool  operator!=(bref const& rhs)const;
    bool  operator< (bref const& rhs)const;
    bool  operator> (bref const& rhs)const;
    bref  operator+ (size_t rhs)     const;
    bref  operator- (size_t rhs)     const;
    bref& operator+=(size_t rhs);
    bref& operator-=(size_t rhs);
    bref& operator++();
    bref  operator++(int);
    bref& operator--();
    bref  operator--(int);
    
    // [i]read_*
    // 
    // read from current position in stream returning a ureg or reg as
    // appropriate for unsigned or signed values
    //
    // i indicates that current position is advanced by the stream read
    //
    // bsize: [0, c_register_bits) the number of bits to read
    //
    // kbit: the packet size for rle reading
    //
    // templeted read allows for correct {reg, ureg} to be called within
    // template instantiation
    //
    template<typename REG_UREG>
    REG_UREG read      (unsigned bsize) const;
    template<typename REG_UREG> 
    REG_UREG iread     (unsigned bsize);
    template<typename REG_UREG>
    REG_UREG iread_rls (unsigned bsize);
    template<typename REG_UREG>
    REG_UREG iread_rle (unsigned kbit);

    // [i]clz
    // scan from current bref and return number of leading zeros prior to binary one
    // This function is defined as long as memory valid until one encountered 
    // iclz variant increments bref past the first one
    unsigned clz()const {bref t(*this); return t.iclz(); }
    unsigned iclz();

    // [i]write[_*]
    // 
    // write to bitstrm a value using one of the above codecs
    //
    // i variants increment bref to the end of the codec
    //
    // bsize for write and iwrite must be at least bref::bsize(value) in size. for
    // rls_write must be exactly bref::bsize_rls<REG_UREG>(value)
    //

    void write_      (ureg signed_or_unsigned_value, unsigned bsize) const;
    void iwrite_     (ureg signed_or_unsigned_value, unsigned bsize);
    template<class REG_UREG>
    void iwrite_rls (REG_UREG value, unsigned bsize);
    template<class REG_UREG>
    void iwrite_rle (REG_UREG value, unsigned kbits = 5);

    // bsize[_*]
    //
    // returns the required or specified bsize for codecs
    template<class REG_UREG>
    static constexpr ureg bsize    (REG_UREG  value);  // a.k.a min_bits
    template<class REG_UREG>
    static constexpr ureg bsize_rls(REG_UREG  value);
    template<class REG_UREG>
    static constexpr ureg bsize_rle(REG_UREG value, unsigned kbit);

    // max/min[_*]
    //
    // returns the maximum or minimum value for codecs given bsize
    template<class REG_UREG>
    static constexpr REG_UREG max(unsigned bsize);
    template<class REG_UREG>
    static constexpr REG_UREG max_rls(unsigned bsize);
    template<class REG_UREG>
    static constexpr REG_UREG max_rle(unsigned bsize, unsigned kbit);    
    template<class REG_UREG>
    static constexpr REG_UREG min(unsigned bsize);
    template<class REG_UREG>
    static constexpr REG_UREG min_rls(unsigned bsize);
    template<class REG_UREG>
    static constexpr REG_UREG min_rle(unsigned bsize, unsigned kbit);

    // _chars
    // 
    // return minimum of bytes(s) necessary to store bsize. NOTE:
    // Relying on minimal bytes may result in memory tools complaining
    // of reading unset bytes, possibly reading beyond owned memory,
    // and race conditions with writing to near objects.  The
    // underscore is to emphasize that the practice is a bit dodgy and
    // best practices are to work on reg boundries e.g. allocation
    // of uregs().
    static ureg _chars(ureg bsize){ return (bsize+__CHAR_BIT__-1)/__CHAR_BIT__;}
    
    // uregs
    // 
    // return number of uregs to store bsize, worth of data 
    static ureg uregs(ureg bsize){
      return (bsize+c_register_bits-1)/c_register_bits;
    }
    
    // print 
    //
    // prints not the value but address of this bref
    //
    // #include "bistrm/print.hpp"
    std::ostream& print(std::ostream& dest)const;
    std::ostream& print()const;

    // subtract
    //
    // in debugger context it my be easier to call functions than operators
    static reg subtract(const bref& lhs, const bref& rhs){
      return c_register_bits*(lhs.m_addr - rhs.m_addr) + lhs.m_off - rhs.m_off;
    }
    
    // merge
    //
    // replace src as defined by mask on dest
    // equivalent to (dest & ~mask) | (src & mask)
    static reg merge(ureg dest, ureg src, ureg mask);
    
  private:
    
    // norm
    // m_addr, m_off represented exactly one way
    // 
    void norm(){
      reg d = (m_off & ~c_register_bit_addr_msk);
      if(!d)
	return;  // ok, nominal case
    
      norm_overflow_underflow(m_off);
    }

    void norm_overflow_underflow(reg d);

    friend ureg popcount(bref, bref);
    friend bref clz   (bref, bref);

  }; // struct bref
  
  inline reg operator-(const bref& lhs, const bref& rhs){
    return bref::subtract(lhs, rhs);
  }

  // in order to generally lighten compile, there is a dependency of #include "bistrm/print.hpp"
  inline std::ostream& operator<< (std::ostream& lhs, const bref& rhs){
    rhs.print(lhs);
    return lhs;
  }

  // algorithms ///////////////////////////////////////////////////////////////
  //
  // also see bitstm utility.hpp for signed/unsigned integer opterations 

  // copy
  // 
  // copys the bits from [begin, end) into the range begining with result
  // returns an bitstream of the end of the destination range
  bref copy(bref begin, bref end, bref result);

  // equal
  // 
  // for [begin, end) return true if equivalent to [second, second + end - begin)  
  bool equal(bref begin, bref end, bref second);

  // clz
  //
  // leading zero run, for [beg, end), not bounded at c_register_bits,
  // return a bref at the first hi bit or else such that return - beg is clz
  bref clz(bref beg, bref end);
  
  // popcount
  //
  // population count of ones in [beg, end), O(ones) complexity
  ureg popcount(bref beg, bref end);

  // print
  //
  // print value of [beg, end) in format bXYZ...
  // #include "bistrm/print.hpp"
  std::ostream&
  print(std::ostream& out, bref beg, bref end);
  
# include "bitstrm/bref_impl.hpp"
  

} // namespace bitstrm

#endif // def'd BITSTRM_BREF_HPP
