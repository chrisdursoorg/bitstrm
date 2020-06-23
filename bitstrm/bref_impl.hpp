// bref_impl.hpp
//
// NOT INTENDED FOR DIRECT REFERENCE -- IMPLEMATION ONLY


// OPERATORS BEGIN
inline ureg
bref::operator*()const { return read_ureg(1); }

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

// READ AND WRITE GENERIC
inline reg
bref::read_reg  (unsigned bsize) const{
  bref t(*this); return t.iread_reg (bsize);
}

inline reg
bref::iread_reg(unsigned bsize){ return iread_as_impl<reg>(bsize); }

inline ureg
bref::iread_ureg(unsigned bsize){ return iread_as_impl<ureg>(bsize); }

inline ureg
bref::read_ureg (unsigned bsize) const{
  bref t(*this); return t.iread_ureg(bsize);
}

template<typename INT_TYPE>
inline INT_TYPE
bref::read_as   (unsigned bsize)const{
  bref t(*this); return t.iread_as<INT_TYPE>(bsize);
}

#if 0
template<typename INT_TYPE> 
inline INT_TYPE
bref::iread_as  (unsigned ){
  static_assert(false, "only reg/ureg types permitted");
}
#endif

template<> reg
inline bref::iread_as(unsigned bsize){ return iread_as_impl<reg>(bsize);}
template<> ureg
inline bref::iread_as(unsigned bsize){
  return iread_as_impl<ureg>(bsize);
}

inline void
bref::write(unsigned bsize, ureg value) const {
  bref t(*this); t.iwrite(bsize, value);
} 

inline ureg
bref::iread_rls(unsigned sbits){
  ureg base = (ureg(1) << sbits) - 1;
  return iread_ureg(sbits) + base;
}

inline ureg
bref::iread_rlp(unsigned prefix_bits){
  ureg suff_bits = iread_ureg(prefix_bits);
  return iread_rls(suff_bits);
}

inline ureg
bref::iread_rlup(){
  ureg run = ilzrun();
  operator++();
  return iread_rls(run);
}

inline void
bref::iwrite_rls(ureg value, unsigned bsize){
  ureg base = (ureg(1) << bsize) - 1;
  assert(base <= value && "verify correct magnitude for value");
  iwrite(bsize, value - base);
}

inline void
bref::iwrite_rlp(ureg value, unsigned prefix_bits){
  ureg suffix_bits = min_bits(value + 1) - 1;
  iwrite(prefix_bits, suffix_bits);
  iwrite_rls(value, suffix_bits);
}

inline void
bref::iwrite_rlup(ureg value){
  ureg suffix_bits = min_bits(value + 1) - 1;
  iwrite(suffix_bits + 1, 1);
  iwrite_rls(value, suffix_bits);
}

// READ AND WRITE GENERIC END


// READ WRITE BEGIN
template<typename _INT_TYPE>
inline _INT_TYPE
bref::iread_as_impl(unsigned bsize){
  assert(bsize <= c_register_bits && "read only defined for regiter bits"); 
  size_t endpos(m_off + bsize);

  if(endpos < c_register_bits && bsize ){
    // capture begining but not through first register
    _INT_TYPE v(endian_adj(*m_addr) << m_off);
    m_off = endpos;
    return v >> (c_register_bits - bsize); 
  } else if(endpos > c_register_bits){
    
    // capture finishing first register and begining on second
    // first registers bits
    _INT_TYPE v(endian_adj(*m_addr) << m_off);
    v >>= (c_register_bits-bsize);
    
    // second registers bits
    ++m_addr;
    m_off = endpos - c_register_bits;
    return v | ((ureg)(endian_adj(*m_addr))) >> (c_register_bits - m_off);

  } else if( endpos == c_register_bits){
    // capture begining and exactly through first register    
    _INT_TYPE v(endian_adj(*m_addr) << m_off);
    v >>= m_off; 
    ++m_addr;
    m_off = 0;
    return v;
  }  
  return 0;  // bitsize == 0 
}

inline void
bref::iwrite(unsigned bsize, ureg value){
  
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

// READ WRITE END


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
bref::ilzrun(){
  
  // first shift left so pos[+0] is MSB
  ureg  cur(endian_adj(*m_addr) << m_off);
  
  unsigned run       = op_clz<ureg>(cur);
  unsigned remaining = unsigned(c_register_bits - m_off);
  
  if( run < remaining ){
    // count complete on current register
    m_off += run;
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

  m_off = locRun;
  norm();
  return run;
}

inline ureg
bref::iread_rle (unsigned kbit){
  assert(kbit > 1 &&
	 "kbit == 1 trivial only able to store 0, kbit == 0 undefined");
  --kbit;
  ureg value = 0;
  ureg next; 
  do {
    next = iread_ureg(1);
    value <<= kbit;
    value |= iread_ureg(kbit);
  } while(next);
  return value;
}

inline reg 
bref::iread_rles(unsigned kbit){
  assert(kbit > 1 &&
	 "kbit == 1 trivial only able to store 0, kbit == 0 undefined");
  --kbit;
  reg      value = 0;
  ureg     next;
  unsigned pay   = 0;
  do{
    next = iread_ureg(1);
    value <<= kbit;
    pay   += kbit;
    value |= iread_ureg(kbit);
  }while(next);

  // now pad left with signed bit
  unsigned header = c_register_bits - pay - 1;
  return (value << header) >> header; 
}

template<class SIGNED_UNSIGNED>
inline void
bref::iwrite_rle(SIGNED_UNSIGNED value, unsigned kbit){
  assert(kbit > 1 &&
	 "kbit == 1 trivial only able to store 0, kbit == 0 undefined");
    
  const unsigned pbit = kbit -1;
  unsigned       part_count  = std::max(((min_bits(value) + pbit - 1) / pbit), ureg(1)); 
  unsigned       shift_right = (part_count-1)*pbit;
  const ureg     term        = (1 << pbit);

  for(; part_count > 1; shift_right -= pbit, --part_count)
    iwrite(kbit, term | ((value >> shift_right) & ~term));

  iwrite(kbit, ((value >> shift_right) & ~term));
}


///////////////////////////////////////////////////////////////////////////////////

// TODO: copy/equal optimization! two things to manage
// 0) begin/end bits,
// 1) intra word allignment
// perhaps half register operation and "bit-alligned" special case 
//
inline 
bref 
copy(bref begin, bref end, bref dest){
  
  reg size = bref::subtract(end, begin);
  for(;size >= c_register_bits; size -= c_register_bits)
    dest.iwrite(c_register_bits, begin.iread_ureg(c_register_bits));

  if(size)
    dest.iwrite(size, begin.iread_ureg(size));
 
  return dest;
}

// optimization! two things to manage 0) begin/end bits, 1) intra byte allignment  
inline 
bool 
equal(bref begin, bref last, bref second){
  
  reg size = bref::subtract(last, begin);
  for(;size >= c_register_bits; size -= c_register_bits)
    if( second.iread_ureg(c_register_bits) !=  begin.iread_ureg(c_register_bits))
      return false;

  if( size && (second.iread_ureg(size) !=  begin.iread_ureg(size)))
    return false;
 
  return true;
}

inline bref
lzrun(bref beg, bref end){
  reg* addr = beg.m_addr;
  // first shift left so pos[+0] is MSB
  ureg  cur(endian_adj(*addr) << beg.m_off);
  
  unsigned  run      = op_clz<ureg>(cur);
  unsigned remaining = unsigned(c_register_bits - beg.m_off);
  
  if( run >= remaining ){
    // must continue to further registers
    run = remaining;
    unsigned locRun;
    do {
      locRun = op_clz<ureg>(endian_adj(*(++addr)));
      run    += locRun;
    } while(locRun == c_register_bits and addr < end.m_addr);  
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

// advance
//
// advance forward to the ord(th) set bit value else end
// end(less) version is well defined when ord count of bits in defined
//
// Performance - perhaps should special case out advance 1 with clz implementation

inline bref advance(bref cur, bref end, ureg cnt){

  for(;cnt != 0; cur = bitstrm::lzrun(++cur, end), --cnt );
  
  return cur;
}

inline bref advance(bref cur, ureg cnt){

  for(;cnt != 0; (++cur).ilzrun(), --cnt );

  return cur;
}

