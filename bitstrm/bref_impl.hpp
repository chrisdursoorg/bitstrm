// bref_impl.hpp
//
// NOT INTENDED FOR DIRECT REFERENCE -- IMPLEMATION ONLY


// OPERATORS BEGIN
inline ureg
bref::operator*()const { return read<ureg>(1); }

inline bool
bref::operator==(const bref& rhs)const{
  return m_addr == rhs.m_addr && m_off == rhs.m_off;
}

inline bool
bref::operator<=(const bref& rhs)const{
  return this->operator<(rhs) || this->operator==(rhs);
}

inline bool
bref::operator>=(const bref& rhs)const{
  return this->operator>(rhs) || this->operator==(rhs);
}

inline bool
bref::operator!=(const bref& rhs)const{ return !(*this == rhs);}

inline bool
bref::operator< (const bref& rhs)const{
  return m_addr < rhs.m_addr || ((m_addr == rhs.m_addr) && m_off < rhs.m_off);
}

inline bool
bref::operator> (const bref& rhs)const{
  return m_addr > rhs.m_addr || ((m_addr == rhs.m_addr) && m_off > rhs.m_off);}

inline bref
bref::operator+ (size_t rhs)const{
  bref r(*this); r.m_off += rhs; r.norm(); return r;
}

inline bref
bref::operator-(size_t rhs)const{
  bref r(*this);
  r.m_off -= rhs; r.norm();
  return r;
}

inline bref&
bref::operator+=(size_t rhs){ m_off += rhs; norm(); return *this;}

inline bref&
bref::operator-=(size_t rhs){ m_off -= rhs; norm(); return *this;}

inline bref&
bref::operator++()          { ++m_off; norm(); return *this; }

inline bref
bref::operator++(int)       { bref r(*this); ++m_off; norm(); return r;}

inline bref&
bref::operator--()          { --m_off; norm(); return *this; }

inline bref
bref::operator--(int)       { bref r(*this); --m_off; norm(); return r;}

//OPERATORS END

////////////////////////////////////////////////////////////////////////////////
// CODEC binary and 2's compliment

template<class REG_UREG>
inline constexpr ureg
bref::bsize(REG_UREG value){
  return min_bits(value);
}

template<>
inline constexpr ureg bref::max<ureg>(unsigned bsize){
  return numeric_limits_unsigned_max(bsize);
}

template<>
inline constexpr reg bref::max<reg>(unsigned bsize){
  return numeric_limits_signed_max(bsize);
}

template<>
inline constexpr ureg bref::min<ureg>(unsigned bsize){
  return numeric_limits_unsigned_min(bsize);
}

template<>
inline constexpr reg bref::min<reg>(unsigned bsize){
  return numeric_limits_signed_min(bsize);
}

inline void
bref::write(ureg value, unsigned bsize) const {
  bref t(*this); t.iwrite(value, bsize);
}

inline void
bref::iwrite(ureg value, unsigned bsize){
  
  assert(bsize <= c_register_bits &&
         "defined for at most register bits");
  // We don't know if value is really a reg/ureg so can't sanity test
  // assert(min_bits(value) >= bsize &&
  //      "value out of range of bsize");

  // TODO: if bitstrm::mask0 can be branchless, we should be able to
  // eliminate this branch too using mask0 below
  if(0 == bsize)
    return;

  int shiftLeft = int(c_register_bits - m_off -bsize);
  
  // one register ?
  if(shiftLeft >= 0){
    // set value
    *m_addr = endian_adj(merge(endian_adj(*m_addr), value << shiftLeft, mask(bsize) << shiftLeft));
    m_off += bsize;
    if(m_off == c_register_bits){
      ++m_addr;
      m_off = 0;
    } 
  } else {
    // divide value to m_addr and m_addr + 1
    // m_addr
    unsigned firstBitSz =  c_register_bits-m_off;
    ureg firstValue = value >> (bsize - firstBitSz);
    *m_addr = endian_adj(merge(endian_adj(*m_addr), firstValue, mask(firstBitSz)));
    m_addr++;
    
    // m_addr + 1
    bsize -= firstBitSz;
    shiftLeft = unsigned(c_register_bits - bsize);
    *m_addr = endian_adj(merge(endian_adj(*m_addr), value << shiftLeft, mask(bsize) << shiftLeft));
    m_off = bsize;
  }
}

template<typename REG_UREG>
inline REG_UREG
bref::read(unsigned bsize) const{
  bref t(*this); return t.iread<REG_UREG>(bsize);
}

template<typename REG_UREG>
inline REG_UREG
bref::iread(unsigned bsize){
  assert(bsize <= c_register_bits && "read only defined for regiter bits"); 
  size_t endpos(m_off + bsize);

  if(bsize == 0 )
      return 0; 

  REG_UREG v(endian_adj(*m_addr) << m_off);
  if(endpos < c_register_bits ){
    // capture begining but not through first register
    m_off = endpos;
    v >>= (c_register_bits - bsize);
    
  } else if(endpos > c_register_bits){
    
    // capture finishing first register and begining on second

    // first registers bits
    v >>= (c_register_bits-bsize);
    
    // second registers bits
    ++m_addr;
    m_off = endpos - c_register_bits;
    v = v | ((ureg)(endian_adj(*m_addr))) >> (c_register_bits - m_off);
  } else if( endpos == c_register_bits){
    // capture begining and exactly through first register    
    v >>= m_off; 
    ++m_addr;
    m_off = 0;
  }
  return v;
}

////////////////////////////////////////////////////////////////////////////////
// CODEC RLS (run length specified)

template<>
inline constexpr ureg
bref::bsize_rls(ureg value){
  return bref::bsize<ureg>(value + 1) - 1;
}

template<>
inline constexpr ureg
bref::bsize_rls(reg value){
  value = (value < 0) ? -value : value;
  return bref::bsize<ureg>(value);
}

template<>
inline constexpr ureg bref::max_rls<ureg>(unsigned bsize){
  ureg base = (ureg(1) << bsize) - 1;
  return bref::max<ureg>(bsize) + base;
}

template<>
inline constexpr reg bref::max_rls<reg>(unsigned bsize){
  if(bsize == 0)
    return 0;
  
  ureg base = ureg(1) << (bsize-1);
  return bref::max<reg>(bsize) + base;
}

template<>
inline constexpr ureg bref::min_rls<ureg>(unsigned bsize){
  ureg base = (ureg(1) << bsize) - 1;
  return bref::min<ureg>(bsize) + base;
}

template<>
inline constexpr reg bref::min_rls<reg>(unsigned bsize){
  if(bsize == 0 )
    return 0;
      
  reg base  = reg(1)-(reg(1) << (bsize-1));
  return bref::min<reg>(bsize) + base;
}

template<>
inline void
bref::iwrite_rls<ureg>(ureg value, unsigned bsize){
  ureg base = (ureg(1) << bsize) - 1;
  assert(base <= value && "verify correct magnitude for value");
  iwrite(value - base, bsize);
}

template<>
inline void
bref::iwrite_rls<reg>(reg value, unsigned bsize){
  reg base = (value < 0) ? (reg(1) - (reg(1) << (bsize-1)))
    : (reg(1) << (bsize-1));
  iwrite(value - base, bsize);
}

template<>
inline ureg
bref::iread_rls<ureg>(unsigned bsize){
  ureg base = (ureg(1) << bsize) - 1;
  return iread<ureg>(bsize) + base;
}

template<>
inline reg
bref::iread_rls<reg>(unsigned bsize){
  if(bsize == 0)
    return 0;
  
  reg relative  = iread<reg>(bsize);
  --bsize;
  reg base = (relative < 0) ? (reg(1)-(reg(1) << bsize)):(reg(1) << bsize);
  return relative + base;
}


////////////////////////////////////////////////////////////////////////////////
// CODEC RLE (run length encoded)

template<class SIGNED_UNSIGNED>
inline constexpr ureg
bref::bsize_rle(SIGNED_UNSIGNED value, unsigned kbit){
  unsigned part = std::max<ureg>(1, ((bsize(value) + kbit - 1 - 1)/(kbit-1)));
  return part*kbit;
}


template<class REG_UREG>
inline constexpr REG_UREG bref::max_rle(unsigned bsize, unsigned kbit){
  assert(kbit > 1 );
  
  unsigned packets    = (bsize + kbit -1)/kbit;
  unsigned tot_packet = packets*kbit;

  return bref::max<REG_UREG>(tot_packet);
}

template<class REG_UREG>
inline constexpr REG_UREG bref::min_rle(unsigned bsize, unsigned kbit){
  assert(kbit > 1 );
  
  unsigned packets    = (bsize + kbit -1)/kbit;
  unsigned tot_packet = packets*kbit;

  return bref::min<REG_UREG>(tot_packet);
}

template<typename REG_UREG>
inline REG_UREG
bref::iread_rle(unsigned kbit){
  assert(kbit > 1 && "kbit < 2 undefined");
  assert(kbit < (c_register_bits-1) && "nonsensical to even approach c_register_bits");
  --kbit;
  ureg next  = iread<ureg>(1);
  ureg value = iread<REG_UREG>(kbit);
  for(;next; ){ 
    value <<= kbit;
    next = iread<ureg>(1);
    value |= iread<ureg>(kbit);
  };
  return value;
}

template<class REG_UREG>
inline void
bref::iwrite_rle(REG_UREG value, unsigned kbit){
  assert(kbit > 1 && "kbit < 2 undefined");
  assert(kbit < (c_register_bits-1) && "nonsensical to even approach c_register_bits");
    
  const unsigned pbit = kbit -1;
  unsigned       part_count  = std::max(((bsize(value) + pbit - 1) / pbit), ureg(1)); 
  unsigned       shift_right = (part_count-1)*pbit;
  const ureg     term        = (ureg(1) << pbit);

  for(; part_count > 1; shift_right -= pbit, --part_count)
    iwrite(term | (value >> shift_right), kbit);

  iwrite(((value >> shift_right) & ~term), kbit);
}

////////////////////////////////////////////////////////////////////////////////


/*static*/
inline reg
bref::merge(ureg dest, ureg src, ureg mask) {return  dest ^ ((dest ^ src) & mask); }

// norm_overflow_underflow
// note not defined for 0
// 
inline void
bref::norm_overflow_underflow(reg d){
 
  // m_off is either negative or greater than c_register_bits

  if( d > 0) {
    // positive case
    m_off   =  (m_off & c_register_bit_addr_msk);
    m_addr += (d >> c_register_bit_addr_sz);
  } else {
    // negative case
    m_off   = c_register_bits + (d  % c_register_bits); 
    m_addr += ( (d - c_register_bits + 1) / c_register_bits);
  }      
}


// clz
//
// Count leading zero's.  Zero returns the number of bits in the register.

template<typename _BASE>
inline _BASE op_clz(_BASE);

template<>
inline uint64_t op_clz<uint64_t>(uint64_t i ){ return i ? __builtin_clzll(i): 64; }

template<>
inline uint32_t op_clz<uint32_t>(uint32_t i ){ return i ? __builtin_clz(i): 32; } 


inline unsigned
bref::iclz(){
  
  // first shift left so pos[+0] is MSB
  ureg  cur(endian_adj(*m_addr) << m_off);
  
  unsigned run       = op_clz<ureg>(cur);
  unsigned remaining = unsigned(c_register_bits - m_off);
  
  if( run < remaining ){
    // count complete on current register
    m_off += (run + 1);
    norm();
    return run;
  }
  
  // must continue to further registers
  run = remaining;
  unsigned locRun;
  do {
    locRun = op_clz<ureg>(endian_adj(*(++m_addr)));
    run  += locRun;
  } while(locRun == c_register_bits);  

  m_off = (locRun + 1);
  norm();
  return run;
}



// TODO: copy optimization! 
// 0) copy x bits src until dst 1/2 register allignment
// 1) shift right src by x, copy rh 1/2 register to dst
// 2) repeat 1 until src.addr == end.addr
// 3) copy src x bits to dst 
//
inline 
bref 
copy(bref begin, bref end, bref dest){
  
  reg bsize = bref::subtract(end, begin);
  for(;bsize >= c_register_bits; bsize -= c_register_bits)
    dest.iwrite(begin.iread<ureg>(c_register_bits), c_register_bits);

  dest.iwrite(begin.iread<ureg>(bsize), bsize);
 
  return dest;
}

// TODO: follow copy allignment optimization
inline 
bool 
equal(bref begin, bref last, bref second){
  
  reg size = bref::subtract(last, begin);
  for(;size >= c_register_bits; size -= c_register_bits)
    if( second.iread<ureg>(c_register_bits) !=  begin.iread<ureg>(c_register_bits))
      return false;

  if( size && (second.iread<ureg>(size) !=  begin.iread<ureg>(size)))
    return false;
 
  return true;
}

inline bref
clz(bref beg, bref end){
  reg* addr = beg.m_addr;
  // first shift left so pos[+0] is MSB
  ureg  cur(endian_adj(*addr) << beg.m_off);
  
  unsigned  run      = op_clz<ureg>(cur);
  unsigned remaining = unsigned(c_register_bits - beg.m_off);
  
  if( run >= remaining and (++addr != end.m_addr)){
    // must continue to further registers
    run = remaining;
    unsigned locRun;
    do {
      locRun = op_clz<ureg>(endian_adj(*(addr)));
      run    += locRun;
    } while(locRun == c_register_bits and ++addr < end.m_addr);  
  }
  
  return std::min(beg + run, end);
}

template<class _BASE> unsigned op_pop_count(_BASE);

template<> inline unsigned op_pop_count<unsigned>(unsigned x)                    { return __builtin_popcount(x);   }
template<> inline unsigned op_pop_count<unsigned long>(unsigned long x)          { return __builtin_popcountl(x);  }
template<> inline unsigned op_pop_count<unsigned long long>(unsigned long long x){ return __builtin_popcountll(x); }

inline 
ureg
popcount(bref cur, bref end){
  ureg count = 0;

  // lead
  for(;cur.m_off != 0; ++cur){
    if(cur == end)
      return count;
    if(*cur)
      ++count;
  }

  // mid -- the optimization -- as long as on a clean mid register use builtin
  for(;cur.m_addr != end.m_addr; ++cur.m_addr)
    count += op_pop_count<ureg>(*cur.m_addr);

  // tail
  for(;cur != end; ++cur)
    if(*cur)
      ++count;
  
  return count;
}






