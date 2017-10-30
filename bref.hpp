// bitstrm/bref.hpp
//
#ifndef BITSTRM_BREF_HPP
#define BITSTRM_BREF_HPP

#include <cassert>
#include <boost/mpl/assert.hpp>
#include "bitstrm/utility.hpp"


// bref(erence)
//
// refers to a location in an allocated bitstrm assuming external
// type/bsize management
//
// two references r0, r1 can consitute a range [r0, r1) if constructed
// from the same base address over contiguous reg castable memory
//
// the default 'pointer' behavior assumes a single bit type, hence
// r1 - r0 is the bitwise distance or bsize, *r0 is the bit value at r0,
// r0++ avavances the reference by 1 bit, etc.
//
// standard access is provided by read/write methods, applying to [0,
// sizeof(reg)*__CHAR_BIT__) (e.g. 63 bits) bit(s) of memory, where
// bits == 0 <=> 0 value for read (or non-operation for write), a
// practical convention for many codecs.


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
    
    // STANDARD OPERATORS
    ureg  operator*()const;
    bref& operator= (const bref& rhs);
    bool  operator==(const bref& rhs)const;
    bool  operator<=(const bref& rhs)const;
    bool  operator>=(const bref& rhs)const;
    bool  operator!=(const bref& rhs)const;
    bool  operator< (const bref& rhs)const;
    bool  operator> (const bref& rhs)const;
    bref  operator+ (size_t rhs)     const;
    bref  operator- (size_t rhs)     const;
    bref& operator+=(size_t rhs);
    bref& operator-=(size_t rhs);
    bref& operator++();
    bref  operator++(int);
    bref& operator--();
    bref  operator--(int);


    // run length specified encoding
    // rls/rlp [r]un [l]ength [s]pecified or [p]refaced codecs:
    //
    // generally integer codecs pack integers into fixed extent bit
    // arrays, hence sequence of unsigned v[3]{0, 1, 2, 3} values can
    // be packed into 0b00,01,10,11 @ 2 bsize per v and having a
    // trivial sub addressing of +{0, 2, 4, 6} bits respectively
    //
    // with rls assume an external entity specifies each bsize (or
    // sub-addressing) of each value.  Here thus significant value
    // information is held externally: again given v[3]{0, 1, 2, 3}
    // <=> external_extents[4]{0,0,1,2,4}, wherein 0 - 0 => 0
    // specifies v[0] entirely, (recall 0 bits <==> 0 value) futher
    // 1 - 0 => 1, specifies v[1] = [1,3), and 2 - 1 => 1 specifes
    // v[2] = [1,3), and 4 - 2 => 2 v[2] = [3,7), thus the remaining
    // rls code for the full v[] is 0b,0,1,00 as it fills in what
    // remains of the value information
    //
    // rlp or size prefaced is a hybrid of fixed and extent
    // approaches, where the max value of v is known as 3, given
    // upper_log(upper_log(3)) => 2 the {prefix}{suffix} format for
    // v[3]{0, 1, 2, 3} is {00}{},{01}{0},{01}{1},{10}{00} or
    // 0b00,010,011,1000 note that with rlp, no external or trivial
    // addressing is possible, one must resort to forward sequential
    // access to read all members
    
    // [i]read_{reg,ureg},iread_rls,iread_rlp}
    // 
    // read from bitstrm bsize bits, interpreting (is_signed<INT_TYPE>)
    // and returning them as appropriate type
    //    + The behavior is defined for bsize < c_register_bits
    //    + read_{reg,ureg}(0) => 0
    //    + i-variant also increments by bsize (optimized variants)
    //    + reg/ureg are signed and unsigned interpretation of underlying bits
    //    + read_as for further template specialization based upon {reg, ureg}
    //    + rls (described above) 
    //    + rlp (descroned above)
    reg  read_reg  (unsigned bsize) const; 
    ureg read_ureg (unsigned bsize) const;
    reg  iread_reg (unsigned bsize);
    ureg iread_ureg(unsigned bsize);
    ureg iread_rls (unsigned specific_bsize);
    ureg iread_rlp (unsigned max_of_prefix = c_register_bit_addr_sz);
    template<typename INT_TYPE>
    INT_TYPE read_as(unsigned bsize) const;
    template<typename INT_TYPE> 
    INT_TYPE iread_as(unsigned );  // iread_as<ureg_or_reg_only>(bsize)

    // write
    // 
    // write to bitstrm bsize bits, encoding as is_signed<INT_TYPE> bits
    //    + The behavior is defined for bsize < c_register_bits
    //    + write(0, any) => non operation
    //    + i variants increment bref
    //    + signed and unsigned writes equivalent
    //      (assuming bsize >= min_bits(value) ), read_as of appropriate type
    //      will restore
    //    + rls (described above) NOTE INCONSISTENCY IN PARAMETER ORDER
    //    + rlp (described above) NOTE INCONSISTENCY IN PARAMETER ORDER
    void write     (unsigned bsize, ureg value) const;
    void iwrite    (unsigned bsize, ureg value);
    void iwrite_rls(ureg value, unsigned bsize_value);
    void iwrite_rlp(ureg value,
                    unsigned max_run_length_bits = c_register_bit_addr_sz);
    static ureg write_rlp_bsize(ureg value,
                                unsigned max_prefix = c_register_bit_addr_sz);
    
    // ilzrun
    // scan from current bref and return number of leading zeros prior to binary 1
    // This function is well defined for leading zeros < sizeof(reg)*CHAR_BIT
    // ilzrun variant increments bref
    unsigned lzrun()const {bref t(*this); return t.ilzrun(); }
    unsigned ilzrun();

    // _chars
    // 
    // return ABSOLUTE minimum of bytes(s) necessary to store
    // bsize. NOTE: Relying on minimal bytes may result in memory
    // tools complaining of reading unset bytes, possibly reading
    // beyond owned memory, and race conditions with writing to close
    // objects.  The underscore is to emphasize that the practice is a
    // bid dodgy and a better practice would be to work on reg
    // boundries e.g. allocation with bitstrm.
    static ureg _chars(ureg bsize){ return (bsize+__CHAR_BIT__-1)/__CHAR_BIT__;}
    
    // uregs
    // 
    // return number of uregs to store bsize, worth of data 
    static ureg uregs(ureg bsize){
      return (bsize+c_register_bits-1)/c_register_bits;
    }
    
    // print 
    //
    // #include "bistrm/print.hpp"
    std::ostream& print(std::ostream& dest)const;
    std::string   print()const;

    // subtract
    //
    // in some debugger context its easier to call functions than operators
    // equivalent to 
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

    template<typename _INT_TYPE> _INT_TYPE iread_as_impl(unsigned bitSz);

  }; // struct bref
  
  inline reg operator-(const bref& lhs, const bref& rhs){
    return bref::subtract(lhs, rhs);
  }

  #if 1
  inline std::ostream& operator<< (std::ostream& lhs, const bref& rhs){
    rhs.print(lhs);
    return lhs;
  }
  #endif

  // copy
  // 
  // copys the bits from [begin, end) into the range begining with result
  // returns an bitstream of the end of the destination range
  bref copy(bref begin, bref end, bref result);

  // equal
  // 
  // for [begin, end) return true if equivalent to [second, second + end - begin)  
  bool equal(bref begin, bref end, bref second);

# include "bitstrm/bref_impl.hpp"
  
} // namespace bitstrm

#endif // def'd BITSTRM_BREF_HPP
