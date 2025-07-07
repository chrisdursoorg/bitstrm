// bitstrm_impl.hpp
//
// IMPLEMENTATION ONLY


// copy_slow
//
// unoptimized copy! A simpler version for testing.
//
inline 
bref 
copy_slow(bref begin, bref end, bref dest){
  
  reg bsize = bref::subtract(end, begin);
  for(;bsize >= c_register_bits; bsize -= c_register_bits)
    dest.iwrite(begin.iread<ureg>(c_register_bits), c_register_bits);

  dest.iwrite(begin.iread<ureg>(bsize), bsize);
 
  return dest;
}

namespace block {

  // block operations assume multi reg[ister] scale user must account for head/tail and
  // determination of alignment
  
  // copy_aligned
  //
  // block copy whole registers from [src, end) to dest, required beg/dest bitwise alignment
  //
  // returns current position of {src, dest} in bref pair
  inline auto
  copy_aligned(reg* src, reg* end, reg* dest){
    dest = std::copy(src, end, dest);
    return std::make_pair<bref, bref>({end, 0},{dest, 0}); 
  }

  // mismatch_aligned
  //
  // find reg wise first mismatching elements [a, last], required a/b must be
  // bitwise alignment
  //
  // returns bref pair of src, tar at or  prior to mismatch else end
  inline auto
  mismatch_aligned(reg* a, reg* end, reg* b){
    auto m = std::mismatch(a, end, b);
    return std::make_pair<bref, bref>({m.first, 0}, {m.second, 0});
  }

  // copy_unaligned
  //
  // block copy bit subset of registers [src, end] that intersect entirely with
  // dest.
  //
  // given (end - src) > 1 and src_off != 0 (is unaligned) copy the src partial
  // register, followed by successive source registers until the last whole dest register
  // is filled
  // 
  // 
  // key: generally |xa|ba|...|by| -> |ab|...|ab, or minimally |xa|by| -> |ab
  // where x and y are discarded non zero length bit sequences
  //
  // returns bref pair of the current src and dest
  inline std::pair<bref, bref>
  copy_unaligned(reg const* src, int src_off, reg const* end, reg* dest_){

    assert(src_off !=0 && "aligned! use block::copy_aligned");
    assert(src_off < int(c_register_bits));
    assert(std::distance(src, end) > 1);
    
    ureg*        dest    = reinterpret_cast<ureg*>(dest_);
    ureg const*  last    = reinterpret_cast<ureg const*>(end)-1;
    ureg const*  cur     = reinterpret_cast<ureg const*>(src);
    ureg         a_width = c_register_bits - src_off;
    ureg         m       = mask0(a_width);

    do {
      *dest =  *cur & m;                   // copy 'a' bits to rhs
      ++cur;
      *dest |= *cur & ~m;                  // copy the 'b' bits to lhs
      *dest =  std::rotr(*dest, a_width);  // swap rot lhs 'a' and rhs 'b' bits
      ++dest;
    } while(cur != last);
	 
    return std::make_pair<bref, bref>
      ({const_cast<reg*>(reinterpret_cast<reg const*>(cur)), src_off},
       {reinterpret_cast<reg*>(dest), 0});
  }

  // mismatch_unaligned
  //
  // Mismatch block test optimized for large matched ranges (O(n)).
  //
  // tar points to ureg with bitstrm offset 0
  // src points to ureg with bitstrm offset (0, c_register_bits)
  // end points to first ureg not entirely full of bitstrm
  //
  // Register by register test equivalence until difference or until cannot test
  // entire register.
  //
  // returns bref pair of src, tar at or  prior to mismatch else end

  inline auto
  mismatch_unaligned(reg const* src, int src_off, reg const* end, reg const* tar){

    assert(src_off !=0 && "aligned! use block::mismatch_aligned");
    assert(src_off < int(c_register_bits));

    // src: |ja0|b0a1|b1a2|...|bxk| -> _src: |ab|[i] and tested with tar[i],
    // where | denotes reg boundry j,k are disregarded bits, i index of combined
    // whole register aligning tar[i]

    ureg  a_width = c_register_bits - src_off;
    ureg  m       = mask0(a_width);
    bool matched  = true;
    while(matched && (src + 1) < end ){
      
      ureg _src  = *src++;
      _src      &=  m;                       // mask 'a' bits to rhs of register
      _src      |= *src & ~m;                 // copy the 'b' bits lhs of register
      _src       = std::rotr(_src, a_width); // swap 'a' @ rhs  with 'b' @ lhs
      matched    = (_src == ureg(*tar));
      tar++;
    }
    
    if(!matched){
      --src;
      --tar;
    }
    
    return std::make_pair<bref, bref>({const_cast<reg*>(src), src_off},
				      {const_cast<reg*>(tar), 0});
  }
} // namespace block

inline
bref
copy(bref cur, bref end, bref dst){

  ureg src_bsize = (end - cur);

  if(src_bsize > 2*c_register_bits){
    // block optimize full register copy

    if(dst.m_off != 0){
      ureg dst_residual = c_register_bits - dst.m_off;
      dst.iwrite(cur.iread<ureg>(dst_residual), dst_residual);
    }

    reg* rend = end.m_addr - 1;  // last full reg
    reg* rcur = cur.m_addr;      // cur part or full reg
    reg* rdst = dst.m_addr;      // dest (full) reg
    
    auto tail = (cur.m_off == 0) ? block::copy_aligned(rcur, rend, rdst)
      : block::copy_unaligned(rcur, cur.m_off, rend, rdst);
    cur = tail.first;
    dst = tail.second;
  }

  // finishing
  while(cur < end){
    ureg chunk = std::min<unsigned>(c_register_bits, end - cur);
    dst.iwrite(cur.iread<ureg>(chunk), chunk);
  }
  
  return dst;
}    


// mismatch optimized to validate large equivalent ranges in linear time
//
// returns first mismatch in [cur, end) else end

inline std::pair<bref, bref>
mismatch(bref cur, bref end, bref tar){

  ureg src_bsize  = bref::subtract(end, cur);
  
  if(src_bsize > 2*c_register_bits){
    // block optimize
    
    // put tar on a full register
    for(; tar.m_off != 0 and cur != end;)
      if(cur.iread<ureg>(1) != tar.iread<ureg>(1))
	return std::make_pair(--cur, --tar);
    
    // rend is last full register
    // rcur is partial or full register
    // rtar is current full register position 
    reg* rend = end.m_addr;
    reg* rcur = cur.m_addr;
    reg* rtar = tar.m_addr;

    auto registers_skip = (cur.m_off == 0)
      ? block::mismatch_aligned(rcur, rend, rtar) :
      block::mismatch_unaligned(rcur, cur.m_off, rend, rtar);

    cur = registers_skip.first;
    tar = registers_skip.second;
  }

  // finish up
  while(cur != end)
    if(cur.iread<ureg>(1) != tar.iread<ureg>(1))
      return std::make_pair(--cur, --tar);
  
  return std::make_pair(cur, tar);
}

inline bool equal(bref begin, bref end, bref target){
    return mismatch(begin, end, target).first == end;
  }
  
inline bref
clz(bref beg, bref end){

  if(beg == end)
    return end;
  
  reg*     addr      = beg.m_addr;
  reg*     eaddr     = end.m_addr;
  ureg     cur       = *addr << beg.m_off; // shift left so pos[0] is MSB
  ureg     run       = op_clz<ureg>(cur);
  unsigned remaining = std::min<unsigned>(c_register_bits - beg.m_off
					  , end - beg);
			   
  if( run >= remaining ){
    // if not finished on register[0]
    run = remaining;
    ureg rrun;
    if( addr != eaddr){
      do {
	++addr;
	if(addr == eaddr){
	  // add last possible leading 0's, memory valid iff off != 0 
	  if(end.m_off)
	    run += std::min<ureg>(op_clz<ureg>(*addr), end.m_off);
	  break;
	}
	// add registers leading zeros
	rrun =  op_clz<ureg>(*addr);
	run  += rrun;
      } while(rrun == c_register_bits);
    }
  }

  return beg + run;  
}
