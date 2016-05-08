// implementation only, do not include directly

// norm_overflow_underflow
// note not defined for 0
// 
inline void bitstrm::norm_overflow_underflow(reg d){
 
  // off_ is either negative or greater than c_register_bits

  if( d > 0) {
    // positive case
    off_ =  (off_ & c_register_bit_addr_msk);
    addr_ += (d >> c_register_bit_addr_sz);
  } else {
    // negative case
    off_   = c_register_bits + (d  % c_register_bits); 
    addr_ += ( (d - c_register_bits + 1) / c_register_bits);
  }      
}

// all variants are implemented as iread_as_impl
template<> reg  inline bitstrm::iread_as(unsigned bitSz){ return iread_as_impl<reg>(bitSz); }
template<> ureg inline bitstrm::iread_as(unsigned bitSz){ return iread_as_impl<ureg>(bitSz); }

reg  inline bitstrm::iread_reg(unsigned bitSz){ return iread_as_impl<reg>(bitSz); }
ureg inline bitstrm::iread_ureg(unsigned bitSz){ return iread_as_impl<ureg>(bitSz); }

template<typename _INT_TYPE>
_INT_TYPE inline bitstrm::iread_as_impl(unsigned bitSz){

  size_t endpos(off_ + bitSz);

  if(endpos < c_register_bits && bitSz ){
    // capture begining but not through first register
    _INT_TYPE v(endian_adj(*addr_) << off_);
    off_ = endpos;
    return v >> (c_register_bits - bitSz); 
  } else if(endpos > c_register_bits){
    
    // capture finishing first register and begining on second
    // first registers bits
    _INT_TYPE v(endian_adj(*addr_) << off_);
    v >>= (c_register_bits-bitSz);
    
    // second registers bits
    ++addr_;
    off_ = endpos - c_register_bits;
    return v | ((ureg)(endian_adj(*addr_))) >> (c_register_bits - off_);

  } else if( endpos == c_register_bits){
    // capture begining and exactly through first register    
    _INT_TYPE v(endian_adj(*addr_) << off_);
    v >>= off_; 
    ++addr_;
    off_ = 0;
    return v;
  }  
  return 0;  // bitsize == 0 
}

inline void bitstrm::iwrite(unsigned bitSz, ureg value){
  
  if(0 == bitSz)
    return;

  int shiftLeft = int(c_register_bits - off_ -bitSz);
  
  // one register ?
  if(shiftLeft >= 0){
    // set value
    *addr_ = endian_adj(merge(endian_adj(*addr_), value << shiftLeft, mask(bitSz) << shiftLeft));
    off_ += bitSz;
    if(off_ == c_register_bits){
      ++addr_;
      off_ = 0;
    } 
  } else {
    // divide value to addr_ and addr_ + 1
    // addr_
    unsigned firstBitSz =  c_register_bits-off_;
    ureg firstValue = value >> (bitSz - firstBitSz);
    *addr_ = endian_adj(merge(endian_adj(*addr_), firstValue, mask(firstBitSz)));
    addr_++;
    
    // addr_ + 1
    bitSz -= firstBitSz;
    shiftLeft = unsigned(c_register_bits - bitSz);
    *addr_ = endian_adj(merge(endian_adj(*addr_), value << shiftLeft, mask(bitSz) << shiftLeft));
    off_ = bitSz;
  }
}

template<typename _BASE>
inline _BASE op_clz(_BASE);

template<>
inline uint64_t op_clz<uint64_t>(uint64_t i ){ return __builtin_clzll(i); }

template<>
inline uint32_t op_clz<uint32_t>(uint32_t i){ return __builtin_clz(i); } 

inline unsigned bitstrm::ilzrun(){
  
  // first shift left so pos[+0] is MSB
  ureg  cur(endian_adj(*addr_) << off_);
  
  unsigned  run = op_clz<ureg>(cur);
  unsigned compliment(unsigned(c_register_bits - off_));
  
  if( run < compliment ){
    // count complete on current register
    off_ += run + 1;
    norm();
    return run;
  }
  
  // must continue to further registers
  run = compliment;
  unsigned locRun;
  do {
    locRun = op_clz<ureg>(endian_adj(*(++addr_)));
    run  += locRun;
  } while(locRun == c_register_bits);  

  off_ = locRun + 1;
  norm();
  return run;
}


// optimization: This could probably be optimzed with two different register sizes
// and shit&mask copying with only the end registers being handled with iread/iwrite.  
inline 
bitstrm 
copy(bitstrm begin, bitstrm last, bitstrm result){
  
  reg size = bitstrm::subtract(last, begin);
  for(;size >= c_register_bits; size -= c_register_bits)
    result.iwrite(c_register_bits, begin.iread_ureg(c_register_bits));

  if(size)
    result.iwrite(size, begin.iread_ureg(size));
 
  return result;
}

// optimization: This could probably be optimzed with two different register sizes
// and shit&mask testing with only the end registers being handled with iread.  
inline 
bool 
equal(bitstrm begin, bitstrm last, bitstrm second){
  
  reg size = bitstrm::subtract(last, begin);
  for(;size >= c_register_bits; size -= c_register_bits)
    if( second.iread_ureg(c_register_bits) !=  begin.iread_ureg(c_register_bits))
      return false;

  if( size && (second.iread_ureg(size) !=  begin.iread_ureg(size)))
    return false;
 
  return true;
}

