// bitstrm/bref.hpp
//
#ifndef BITSTRM_BREF_HPP
#define BITSTRM_BREF_HPP

#include <cassert>
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
// sizeof(reg)*__CHAR_BIT__) (e.g. 64 bits) bit(s) of memory, where
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

    // integer encodings
    //  clz    count leading zeros
    //         (e.g. {1, 01, 001, 0001} -> {0, 1, 2, 3})
    //
    //  rls    run length (externally) specified, given a bsize of unsigned 
    //         integer infer its magnitude (msb) encoding the mantissa optimally
    //         (e.g. {'', '0', '00', '11'} -> {0, 1, 3, 6})
    //
    //  rlup   run length unary encode, (efficeint with very small numbers)
    //         clz read the preface the rls
    //         (e.g. {1|, 01|0, 001|00, 001|11} -> {0, 1, 3, 6})
    //
    //  rle    run length encode, (next prefix) kbit packet size, kbit > 1
    //         (e.g. k:5 {0|0000, 1|1100^0|0001} -> {0, 193})
    //
    //  rles   signed integer run length encode, (next prefix) kbit packet size
    //         (e.g. k:5 {0|0000, 1|1100^0|0001} -> {0, -63})
    //
    //  rlp    run length prefix, first read prefix_bsize as kbit unsigned
    //         integer to determine bsize then rls read the numbe
    //         (e.g. {000000|, 000001|0, 000010|00, 000010|11}  -> {0, 1, 3, 6})
    //
    // run length specified maps integers with specified or listed
    // as in [prefix]{(2^(*prefix)-1)} bits, values of 
    // [0, 2^(total_bits-prefix) -2]
    //
    // See table at bottom, rlp_bsize and rlup_bsize for ranges of values
    // acheivable for differing number of bits

    
    // [i]read_{reg,ureg},iread_rls,iread_rlp, iread_rlup}
    // 
    // read from bitstrm bsize bits, interpreting (is_signed<INT_TYPE>)
    // and returning them as appropriate type
    //    + The behavior is defined for bsize < c_register_bits
    //    + read_{reg,ureg}(0) => 0
    //    + i-variant also increments by bsize (optimized variants)
    //    + reg/ureg are signed and unsigned interpretation of underlying bits
    //    + read_as for further template specialization based upon {reg, ureg}
    //    + rls  (described above) 
    //    + rlp  (descroned above)
    //    + rlup (described above)
    //    + rle  (described above)
    //    + rles (descrined above)
    reg  read_reg  (unsigned bsize) const; 
    ureg read_ureg (unsigned bsize) const;
    reg  iread_reg (unsigned bsize);
    ureg iread_ureg(unsigned bsize);
    ureg iread_rls (unsigned specific_bsize);
    ureg iread_rlp (unsigned prefix_bsize = c_register_bit_addr_sz);
    ureg iread_rlup();
    ureg iread_rle (unsigned kbit);
    reg  iread_rles(unsigned kbit);

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
    //      signed and unsigned writes equivalent assuming iread_rle and iread_rles
    //      called appropriately
    //    + rlp (described above) NOTE INCONSISTENCY IN PARAMETER ORDER
    //    + rlup (described above)
    //     
    void write      (unsigned bsize, ureg value) const;
    void iwrite     (unsigned bsize, ureg value);
    void iwrite_rls (ureg value, unsigned bsize_value);
    void iwrite_rlp (ureg value, unsigned max_bsize = c_register_bit_addr_sz);
    void iwrite_rlup(ureg value);
    template<class SIGNED_UNSIGNED>
    void iwrite_rle (SIGNED_UNSIGNED value, unsigned kbits = 5);

    // ilzrun
    // scan from current bref and return number of leading zeros prior to binary one
    // This function is defined as long as memory valid until one encountered 
    // ilzrun variant increments bref to first one
    unsigned lzrun()const {bref t(*this); return t.ilzrun(); }
    unsigned ilzrun();


    template<class SIGNED_UNSIGNED>
    static ureg rle_bsize(SIGNED_UNSIGNED value, unsigned kbit){
      if(value == 0)
	return kbit;
      unsigned part = ((min_bits(value) + kbit - 1 - 1)/(kbit -1));
      return part*kbit;
    }
    
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
    // prints not the value but address of this bref
    //
    // #include "bistrm/print.hpp"
    std::ostream& print(std::ostream& dest)const;
    std::ostream& print()const;

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

    friend ureg popcount(bref, bref);
    friend bref advance (bref, bref, ureg);
    friend bref advance (bref, ureg);
    friend bref lzrun   (bref, bref);

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

  // advance
  //
  // return advanced bref to the ord(th) value by successive calls to lzrun
  // end(less) version is well defined when ord count of bits in range
  bref advance(bref beg, bref end, ureg ord);
  bref advance(bref beg, ureg ord);

  // copy
  // 
  // copys the bits from [begin, end) into the range begining with result
  // returns an bitstream of the end of the destination range
  bref copy(bref begin, bref end, bref result);

  // equal
  // 
  // for [begin, end) return true if equivalent to [second, second + end - begin)  
  bool equal(bref begin, bref end, bref second);


  // lzrun
  //
  // leading zero run, for [beg, end) return a bref to the first one encountered
  // else end
  bref lzrun(bref beg, bref end);
  
  
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
  
  // Illustration and Python program enumerating the ranges of
  // p[refix]bits and t[otal]bits and also m[antissa]bits and
  // the implicite base value associated the value of pbits

  
  // hformat = "{:>7s} ".join("      ")
  // fformat = "{:7d} ".join("      ")
  
  // def first2last2(lst):
  //     len = lst.__len__()
  //     if len > 7 :
  //         first2last2(lst[0:2])
  //         print("{:^54s}".format("..."))
  //         print(lst[int(len/2)])
  //         print("{:^54s}".format("..."))
  //         first2last2(lst[-2:])
  //     else:
  //         for i in lst:
  //             print(i)


  // print(hformat.format("pbits", "mbits", "tbits", "base", "max"))
  // for pbits in range(1,7):
  //     lines = []
  //     for mbits in range(0, 1 << pbits):
  //         tbits = mbits + pbits
  //         base = (1 << mbits) -1
  //         max = 2*base
  //         lines.append(fformat.format(pbits, mbits, tbits, base, max))

  //     first2last2(lines)
  
   // pbits    mbits    tbits     base      max  
   //     1        0        1        0        0  
   //     1        1        2        1        2  
   //     2        0        2        0        0  
   //     2        1        3        1        2  
   //     2        2        4        3        6  
   //     2        3        5        7       14  
   //     3        0        3        0        0  
   //     3        1        4        1        2  
   //                       ...                          
   //     3        4        7       15       30  
   //                       ...                          
   //     3        6        9       63      126  
   //     3        7       10      127      254  
   //     4        0        4        0        0  
   //     4        1        5        1        2  
   //                       ...                          
   //     4        8       12      255      510  
   //                       ...                          
   //     4       14       18    16383    32766  
   //     4       15       19    32767    65534  
   //     5        0        5        0        0  
   //     5        1        6        1        2  
   //                       ...                          
   //     5       16       21    65535   131070  
   //                       ...                          
   //     5       30       35  1073741823  2147483646  
   //     5       31       36  2147483647  4294967294  
   //     6        0        6        0        0  
   //     6        1        7        1        2  
   //                       ...                          
   //     6       32       38  4294967295  8589934590  
   //                       ...                          
   //     6       62       68  4611686018427387903  9223372036854775806  
   //     6       63       69  9223372036854775807  18446744073709551614 
  
  
} // namespace bitstrm

#endif // def'd BITSTRM_BREF_HPP
