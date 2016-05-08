#ifndef BITSTRMHPP_
#define BITSTRMHPP_

#include "bitstrm/reg.hpp"
#include <boost/mpl/assert.hpp>
#include <iosfwd>

namespace bitint {

    // bitstrm is the fundemental memory reference allowing for sub-register integers or arbitrary size.  Keeping the underlying memory address (addr_) 
    // and the current offset within that address (off_).  Though agnostic to the bitSz and signed/unsigned ness of the integers bitstrm provides the functions 
    // necessary to encode/decode fixed length integers and lz(leading zero) run lenght values. 
    //
    // See bitstrm::endian_adj and bitstrm:reg to compiled to address endianness and different register types respectively and bitstrm::bint_iter for iterator interface
    // 
    //   // Example:

    //     typedef  std::vector<int> containerType;
    //     int initValues[] = {1,2,3,4,-4,5,6,4,5,6,7,192,323,3223,121,123};
    //     containerType container(initValues, initValues + sizeof(initValues)/sizeof(int));
    //     std::vector<char> buf(1024);  // a clean way to create contiguous memory buffer is to use std::vector
    //     bitstrm p(&buf.front());
    //     bitstrm p0(p);           // save off a copy of your origin (or you could just rereference buf.front())
    
    //     unsigned bitSz = min_bits(container.begin(),container.end());
    //     for_each(container.begin(), container.end(), [bitSz,&p](containerType::value_type value) { p.iwrite(bitSz,value); });
  
    //    std::cout << "we've copied " << container.size() << " values of bitSz " << bitSz << " taking a total of " << p - p0 << " bits." << std::endl;
  
    //    containerType::const_iterator b = container.begin();
    //    containerType::const_iterator e = container.end();
    //    p = p0;
    //    std::cout << "REStoring VALues\nVAL\tRES"  << std::endl;
    //    for(; b != e; ++b){
    //       std::cout << *b << "\t" << p.iread_reg(bitSz) << endl;
    //    }
    // we've copied 16 values of bitSz 13 taking a total of 208 bits.
    // REStoring VALues
    // VAL	RES
    // 1	1
    // 2	2
    // 3	3
    // 4	4
    // -4	-4
    // 5	5
    // 6	6
    // 4	4
    // 5	5
    // 6	6
    // 7	7
    // 192	192
    // 323	323
    // 3223	3223
    // 121	121
    // 123	123

    struct bitstrm {
    private:
      reg* addr_;
      reg  off_;
    
    public:
      inline bitstrm(){};
      inline bitstrm(const bitstrm& rhs): addr_(rhs.addr_), off_(rhs.off_){}
      inline bitstrm(char* addr): addr_(reinterpret_cast<reg*>(addr)), off_(0){}
      inline bitstrm(char* addr, int off): addr_(reinterpret_cast<reg*>(addr)), off_(off){ norm(); }

      // perhaps this should be a reg& to internal copy???
      inline reg     operator*()const{ return endian_adj(*addr_) & (1 << off_); }    
 
      inline bitstrm& operator=(const bitstrm& rhs) { addr_ = rhs.addr_; off_ = rhs.off_; return *this; }
      inline bool    operator==(const bitstrm& rhs)const{ return addr_ == rhs.addr_ && off_ == rhs.off_;}
      inline bool    operator<=(const bitstrm& rhs)const{ return this->operator<(rhs) || this->operator==(rhs); }
      inline bool    operator>=(const bitstrm& rhs)const{ return this->operator>(rhs) || this->operator==(rhs); }
      inline bool    operator!=(const bitstrm& rhs)const{ return !(*this == rhs);}
      inline bool    operator<(const bitstrm& rhs)const{ return addr_ < rhs.addr_ || ((addr_ == rhs.addr_) && off_ < rhs.off_);}
      inline bool    operator>(const bitstrm& rhs)const{ return addr_ > rhs.addr_ || ((addr_ == rhs.addr_) && off_ > rhs.off_);}
      inline bitstrm  operator+(size_t rhs)const{ bitstrm r(*this); r.off_ += rhs; r.norm(); return r; }
      inline bitstrm  operator-(size_t rhs)const{ bitstrm r(*this); r.off_ -= rhs; r.norm(); return r; }
      inline bitstrm& operator+=(size_t rhs){ off_ += rhs; norm(); return *this; }
      inline bitstrm& operator-=(size_t rhs){ off_ -= rhs; norm(); return *this; }
      inline bitstrm& operator++(){ ++off_; norm(); return *this; }
      inline bitstrm  operator++(int){ bitstrm r(*this); ++off_; norm(); return r;}
      inline bitstrm& operator--(){ --off_; norm(); return *this; }
      inline bitstrm  operator--(int){ bitstrm r(*this); --off_; norm(); return r;}

      // read_reg/ureg
      // 
      // read from bitstrm bitSize bits, interpreting (is_signed<INT_TYPE>) and returning them as appropriate type
      //    + The behavior is defined for bitSz <= c_register_bits
      //    + read_(0) => 0
      //    + iread_ variant increments 
      //    + reg/ureg variants are signed and unsigned respectively
      //    + read_as convienient for further template specialization based upon {reg, ureg}
      reg  inline read_reg  (unsigned bitSz) const{ bitstrm t(*this); return t.iread_reg (bitSz); } 
      ureg inline read_ureg (unsigned bitSz) const{ bitstrm t(*this); return t.iread_ureg(bitSz); }
      reg         iread_reg (unsigned bitSz);
      ureg        iread_ureg(unsigned bitSz);
      template<typename INT_TYPE> INT_TYPE inline read_as(unsigned bitSz) const{ bitstrm t(*this); return t.iread_as<INT_TYPE>(bitSz); } 
      template<typename INT_TYPE> INT_TYPE inline iread_as(unsigned ){ BOOST_MPL_ASSERT_MSG(sizeof(INT_TYPE)==0, ONLY_TYPES_REG_UREG_ALLOWED, (void) ); }    
    
      // write
      // 
      // write to bitstrm bitSize bits, encoding as is_signed<INT_TYPE> bits
      //    + The behavior is defined for c_register_bits
      //    + iwrite(0, any) => NOOP
      //    + iwrite variant increments bitstrm
      //    + signed and unsigned writes equivalent (assuming bitSz >= min_bits(value) ), value is masked, read_as of appropriate variant should restore
      void inline write(unsigned bitSz, ureg value) const { bitstrm t(*this); t.iwrite(bitSz, value); } 
      void iwrite(unsigned bitSz, ureg value);

      //  ilzrun
      // scan from current pos and return number of leading zeros prior to binary 1
      // This function is well defined for leading zeros <= sizeof(reg)*CHAR_BIT
      // ilzrun variant increments bitstrm
      inline unsigned lzrun()const {bitstrm t(*this); return t.ilzrun(); }
      unsigned ilzrun();
    
      // print 
      //
      void print(std::ostream& dest)const;
      std::string print()const;

      // subtract
      //
      // because in debuggers its easier to call functions than operators
      inline static reg subtract(const bitstrm& lhs, const bitstrm& rhs){
	return c_register_bits*(lhs.addr_ - rhs.addr_) + lhs.off_ - rhs.off_;
      }
    
      // chars
      // 
      // return number of char(s) necessary to store bitstrm_sz
      inline static ureg chars(ureg bitsz){ return (bitsz + __CHAR_BIT__ -1 )/__CHAR_BIT__; }
    
      // mask
      //
      // return bitSz (when <= c_register_bits) 1 bits
      inline static ureg mask(unsigned bitSz){ return ureg(-1) >> (c_register_bits - bitSz); }

      // merge
      // equivalent to result of (dest & ~mask) | (src & mask)
      inline static reg merge(ureg dest, ureg src, ureg mask) { return  dest ^ ((dest ^ src) & mask); }
    
    private:
      // norm
      // addr_ and off_ should be adjusted such that one bit addr_ess would be represented exactly one way
      // 
      inline void norm(){
	reg d = (off_ & ~c_register_bit_addr_msk);
	if(!d)
	  return;  // ok, nominal case
    
	norm_overflow_underflow(off_);
      }

      void norm_overflow_underflow(reg d);
      template<typename _INT_TYPE> _INT_TYPE iread_as_impl(unsigned bitSz);

    };  // struct bitstrm

    // Though a bitstrm is akin to a pointer the underlying memory can be thought of as
    // an abstract object whatever its contents are meant to represent, hence copy and
    // equals.  PERFORMANCE BEWARE: In generalizing, implementation must assume that registers 
    // are not alligned, hence these operations would be SIGNIFICANTLY SLOWER than a memory
    // copy and compare

    // copy
    // 
    // copys the bits from [begin, end) into the reange begining with result
    // returns an bitstream of the end of the destination range
    inline bitstrm copy(bitstrm begin, bitstrm end, bitstrm result);

    // equal
    // 
    // for [begin, end) return true if equivalent to [second, second + end - begin)  
    inline bool equal(bitstrm begin, bitstrm end, bitstrm second);

#include "bitstrm/bitstrm_impl.hpp"

    inline reg operator-(const bitstrm& lhs, const bitstrm& rhs){
      return bitstrm::subtract(lhs, rhs);
    }

    inline std::ostream& operator<< (std::ostream& lhs, const bitstrm::bitstrm& rhs){
      rhs.print(lhs);
      return lhs;
    }


} // namespace bitint  


#endif // def'd BITSTRMHPP_
