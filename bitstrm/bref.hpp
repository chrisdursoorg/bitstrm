// bitstrm/bref.hpp
//
#ifndef BITSTRM_BREF_HPP
#define BITSTRM_BREF_HPP

#include <cassert>
#include <bitstrm/reg.hpp>
#include <bitstrm/utility.hpp>
#include <iosfwd>

#ifdef USE_ENDIAN
#error "bswap adjustment supporting endian addressing has been removed"
#endif


// bref(erence)
//
// Refers to a location in an allocated bitstrm assuming external
// type/bsize management abstracting but working on reg boundary.
//
// With 'pointer' or 'iterator' like behavior (defaults to a single bit),
// r1 - r0 is the bitwise distance or bsize, *r0 is the
// bit value at r0, r0++ advances the reference by 1 bit, etc.
//
// Two bref r0, r1 can constitute a range [r0, r1) if constructed from
// the same base address over contiguous reg cast-able memory.
//
// Milti-bit access allows for the mapping of [0,
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
//         May be either signed or unsigned value.    
//         (e.g. k:5 {0|0000, 1|1100^0|0001} -> {0, 193})
//

namespace bitstrm {

  struct bref {
    
  private:
    reg* m_addr;
    reg  m_off;
    
  public:
    bref(){};
    bref(const bref& rhs)   : m_addr(rhs.m_addr), m_off(rhs.m_off){}
    bref(reg* buf)          : m_addr(buf), m_off(0){}
    bref(reg* buf, int off) : m_addr(buf), m_off(off){
      norm();}
    constexpr bref(bref&& rhs) = default;
    
    // STANDARD OPERATORS
    bref& operator= (const bref& rhs) = default;
    bref& operator= (bref&& rhs) = default;
    bool  operator==(bref const& rhs)const;
    bool  operator<=(bref const& rhs)const;
    bool  operator>=(bref const& rhs)const;
    bool  operator!=(bref const& rhs)const;
    bool  operator< (bref const& rhs)const;
    bool  operator> (bref const& rhs)const;
    bref  operator+ (reg rhs)     const;
    bref  operator- (reg rhs)     const;
    bref& operator+=(reg rhs);
    bref& operator-=(reg rhs);
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
    //
    // Termination must occur in valid memory.
    //
    // iclz advances to the first 1, return the number of leading zeros
    // advanced past
    ureg clz()const {bref t(*this); return t.iclz(); }
    ureg iclz();

    // [i]write[_*]
    // 
    // write to bitstrm a value using one of the above codecs
    //
    // i variants increment bref to the end of the codec
    //
    // bsize for write and iwrite must be at least bref::bsize(value) in size. for
    // rls_write must be exactly bref::bsize_rls<REG_UREG>(value)
    //
    void write      (ureg signed_or_unsigned_value, unsigned bsize) const;
    void iwrite     (ureg signed_or_unsigned_value, unsigned bsize);
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

    // uregs
    // 
    // return number of uregs to store bsize, worth of data 
    static ureg uregs(ureg bsize){
      return (bsize+c_register_bits-1)/c_register_bits;
    }
    
    // print 
    //
    // print address of this bref in format 0xXX..X +[off] or '0xNullptr +0'
    //
    // #include "bistrm/print.hpp"
    std::ostream& print(std::ostream& dest)const;
    std::ostream& print()const;

    // subtract
    //
    // in debugger context it my be easier to call functions than operators
    static std::size_t subtract(const bref& lhs, const bref& rhs){
      return c_register_bits*(lhs.m_addr - rhs.m_addr) + lhs.m_off - rhs.m_off;
    }
    
    // merge
    //
    // replace src as defined by mask on dest
    // equivalent to (dest & ~mask) | (src & mask)
    static reg merge(ureg dest, ureg src, ureg mask);
    
  private:
    
    // norm
    // m_addr, m_off such that m_off is bounded to [0, c_register_bits)
    // 
    void norm(){
      // an offset > c_register_bit_addr_msk or < 0 must be normalized
      if(!(m_off & ~c_register_bit_addr_msk))
	return;  // ok, passes as normal
      if(m_off > 0){
	// too large
	m_addr += (m_off / (reg)(c_register_bits));
	m_off   = (m_off % (reg)(c_register_bits));
      } else {
	// too small (any negative value)
	reg w = (m_off + 1) / c_register_bits;
	reg r = (m_off + 1) % c_register_bits;

	m_off   = (c_register_bits - 1 + r);
	m_addr -= (1 - w);
      }    
    }

    friend ureg popcount(bref, bref);
    friend bref clz   (bref, bref);
    friend bref copy(bref begin, bref end, bref destination);
    friend std::pair<bref, bref> mismatch(bref begin, bref end, bref target);

  }; // struct bref

  
  inline std::size_t operator-(const bref& lhs, const bref& rhs){
    return bref::subtract(lhs, rhs);
  }

  // in order to generally lighten compile, there is a dependency of
  // #include "bistrm/print.hpp"
  inline std::ostream& operator<< (std::ostream& lhs, const bref& rhs){
    rhs.print(lhs);
    return lhs;
  }

// also see bitstrm utility.hpp for signed/unsigned integer operations

#include <bitstrm/bref_impl.hpp>

} // namespace bitstrm

#endif // def'd BITSTRM_BREF_HPP
