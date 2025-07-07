// bref_unittest.cpp

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "BitStreamBref"

#include <boost/test/unit_test.hpp>
#include "bitstrm/bref.hpp"
#include "bitstrm/alloced_bref.hpp"
#include "bitstrm/print.hpp"
#include "bitstrm/utility.hpp"
#include <limits>
#include <iostream>
#include <sstream>
#include <set>
#include <random>
#include <cmath>

using namespace boost::unit_test;
using namespace bitstrm;
using namespace std;

// print_human_bits
//
// formats bits as human-readable string
template<typename NUMERIC_TYPE>
std::ostream& print_human_bits(std::ostream& out, NUMERIC_TYPE bits){
    
  unsigned long long  bytes = bits/8;

  if(bytes < 2){
    out << bits << " bits";
  } else if(bytes < 1024){
    unsigned long long whole_bytes = bits/8;
    out << whole_bytes << " bytes";
    NUMERIC_TYPE remain = (bits - (whole_bytes*8));
    if(remain )
      out << " " << remain << " bits";
  } else if(bytes < 1024*1024){
    out << double(bits) / (8*1024) << " kbytes";
  } else if(bytes < 1024*1024*1024){
    out << double(bits) / (8*1024*1024) << " mbytes";
  } else {
    out << double(bits)/(double(8)*1024*1024*1024) << " gbytes";
  }
    
  return out;
}


BOOST_AUTO_TEST_CASE(g_non_ureg_reg_types_with_read){
    // stated in interface read was to be done on reg/ureg type but implemented generally resulting in errors
    // with other types. Internally the type is now reg/ureg correcting the general case.

  alloced_bref src(40);
  src.write(42, 12);
  BOOST_CHECK(src.read<ureg>(12) == 42);
  alloced_bref dest(60);
  bref end = bitstrm::copy(src, src + src.bsize(), dest);
  BOOST_CHECK((end - dest) == 40);
  BOOST_CHECK(dest.read<int>(12) == 42);
  BOOST_CHECK(dest.read<float>(12) == 42.);
}


BOOST_AUTO_TEST_CASE(g_min_size_write_and_restore)
{
  BOOST_TEST_MESSAGE("test min_bits() with bitstrm::ref");
  vector<reg> buf(bref::uregs(8)*CHAR_BIT);
  
  for(int i = 1024*1024; i > -1024*2024; --i){

    bref w(&buf.front());
    bref r(w);
    unsigned mbits = min_bits(i);

    // this many bits correctly stores the value
    w.write(mask0(mbits) & i, mbits);
    reg restore = r.read<reg>(mbits);
    if( i != restore)
      BOOST_CHECK_MESSAGE(i == restore, "i: " << i << " restore: " << restore
			  << " does not match");
    // any less bits fails
    if(mbits){  // not zero!
      --mbits;
      w.write(mask0(mbits) & i, mbits);
      reg broken = r.read<reg>(mbits);
      if(i == broken)
	BOOST_CHECK_MESSAGE(i != broken, "i: " << i << " broken: " << broken
			    << " should not match");
    }
  } 
}

BOOST_AUTO_TEST_CASE(arbitary_table_and_reset){
  alloced_bref strm(1024*17);
  ureg i = 0;
  const unsigned mask17bit = mask(17);
  BOOST_TEST_MESSAGE("fill up a 17 bit table");
  while(!(mask17bit & i))
    strm.iwrite(i++, 17);

  // as a bitstrm (instead of simple bref) it can be reset
  strm.reset();

  BOOST_TEST_MESSAGE("verify 17 bit table");
  i = 0;
  while(!(mask17bit & i))
    BOOST_CHECK(strm.iread<ureg>(17) == i++);
  
}

BOOST_AUTO_TEST_CASE(late_allocate_and_swap){
  alloced_bref strm;
  strm.resize(1024*17);
  ureg i = 0;
  const unsigned mask17bit = mask(17);
  BOOST_TEST_MESSAGE("fill up a 17 bit table");
  while(!(mask17bit & i))
    strm.iwrite(i++, 17);

  // a quick swap operation moves the underlying bits to strm2
  alloced_bref strm2;
  strm2.swap(strm);
  // keep track of curent reference position as it would have
  // moved too
  bref end = strm2;
  strm2.reset();
  // verify that extent given by end was correct
  BOOST_CHECK(end - strm2 == 17*i);

  BOOST_TEST_MESSAGE("verify 17 bit table");
  i = 0;
  while(!(mask17bit & i))
    BOOST_CHECK(strm2.iread<ureg>(17) == i++);
}

// This test of a simple codec to save space, for msg lower case chars
// and spaces, and we map space to 'z' + 1 in bottom 5 bits
BOOST_AUTO_TEST_CASE(g_hello_world2)
{
  const char   msg[] = "hello world";
  const int    sz = strlen(msg);

  vector<reg> buf( bref::uregs(5*sz));

  bref write(&buf.front());
  bref read(write);
  const char c_five_bits = (( 1<<5 ) - 1 );
  for_each( msg, msg + sz, [&write](const char v){ 
      if(v != ' ') 
        write.iwrite(v & c_five_bits, 5);
      else 
        write.iwrite(('z' + 1) & c_five_bits, 5);});

  BOOST_CHECK((write - read) == 5 * sz); // its now compresed 8:5  or  .625 
  
  // check the decoded values
  for_each( msg, msg + sz, [&read](const char v){ 
      char decoded = read.iread<ureg>(5) | 0x60;
      if(decoded == 'z' + 1)
        decoded = ' ';
      BOOST_CHECK(decoded  == v );}   );
}

BOOST_AUTO_TEST_CASE(g_bitstrm_ops){  

  reg  untouched;
  bref p(&untouched);
  bref o = p++;
  
  BOOST_CHECK( o < p );
  BOOST_CHECK( !(o > p));
  BOOST_CHECK(o != p);
  BOOST_CHECK(!(o == p));
  BOOST_CHECK((p - o) == 1);
  BOOST_CHECK((++p - o) == 2);
  p += 100;
  BOOST_CHECK((p - o) == 102);
  p++;
  BOOST_CHECK((p - o) == 103);
  BOOST_CHECK((--p - o ) == 102);
  p--;
  BOOST_CHECK((p - o) == 101);
  p -= 100;
  BOOST_CHECK((p - o) == 1);
}

int  three_bit_signed_numbers[] = {3, 2, 1, 0, -1, -2, -3, -4};
int* three_bit_signed_numbers_e = three_bit_signed_numbers
  + sizeof(three_bit_signed_numbers)/sizeof(int);

BOOST_AUTO_TEST_CASE(g_bitstrm_read_and_write){
  vector<reg> buf(256);
  const char  sentry = 42;
  buf.back()  = sentry;
  
  bref p(&buf.front(),2);
  bref po(p);

  const int* bv = three_bit_signed_numbers;
  const int* ev = bv+sizeof(three_bit_signed_numbers)/sizeof(unsigned);    
  
  for (int bits = 4; bits <= 64; ++bits, p = po){
      
    for_each(bv, ev, [bits,&p](int v) { p.iwrite(v, bits);} );
      BOOST_CHECK(bref::uregs(p - po) <=  (buf.size() -1 ));
      for_each(bv, ev, [bits,&p](int v) { p.iwrite(v, bits);} );
      BOOST_CHECK(bref::uregs(p - po) <=  (buf.size() -1 ));
      p = po;
      for_each(bv, ev, [bits,&p](int v) { 
          reg restore(p.iread<reg>(bits));  
          BOOST_CHECK( restore == v);
        });
  }

  BOOST_CHECK(buf.back() == sentry);
}


BOOST_AUTO_TEST_CASE(g_bitstrm_read_write_arbitrary_depths){
  vector<reg> buf(128);
  const char   sentry = 42;
  buf.back()   = sentry;

  bref p(&buf.front(),2);
  bref po(p);

  const int* bv = three_bit_signed_numbers;
  const int* ev = bv+sizeof(three_bit_signed_numbers)/sizeof(unsigned);    
   
  for (size_t bits = 4; bits <= 64; ++bits, p = po){

    // write
    for_each(bv, ev, [bits,&p](int v) { p.iwrite(v, bits);} );
    BOOST_CHECK(size_t(bits*(ev-bv)) == (p-po));
    BOOST_CHECK(bref::uregs(p - po) <=  buf.size());
  
    // restore
    p = po;
    for_each(bv, ev, [bits,&p](int v) { 
        reg restore(p.iread<reg>(bits));  
        BOOST_CHECK(v == restore);
      } );
  }
  BOOST_CHECK(buf.back() == sentry);
}

BOOST_AUTO_TEST_CASE(g_clz_){
  ureg v = 0; v = ~v; // 0b111..1
  BOOST_CHECK( __builtin_clzll(v)  == 0);
  BOOST_CHECK( op_clz<uint64_t>(v) == 0);
  v = (~(v>>1));      // 0b100..0
  BOOST_CHECK( __builtin_clzll(v)  == 0);
  BOOST_CHECK( op_clz<uint64_t>(v) == 0);
  v >>= 1;
  BOOST_CHECK( __builtin_clzll(v)  == 1);
  BOOST_CHECK( op_clz<uint64_t>(v) == 1);
  v=0;
  // BOOST_CHECK( __builtin_clzll(v) == 64);  Undefined!
  BOOST_CHECK( op_clz<uint64_t>(v) == 64);
}

BOOST_AUTO_TEST_CASE(g_clz_bitstrm){
  ureg max_count = 11;
  ureg bits_for_test   = (max_count + 1)*(max_count + 2)/2;
  ureg magic           = 42;
  ureg bits_for_sentry = (min_bits(magic));
  alloced_bref buf( bits_for_test + bits_for_sentry);
  bref po(buf);

  for(int i = max_count + 1; i > 0 ; --i)
    buf.iwrite(1,i);  // write i-1 leading zeros
  buf.iwrite(magic, bits_for_sentry);

  for( int v = max_count; v != -1; --v){
    int run = po.iclz();
    ++po;
    BOOST_CHECK_MESSAGE( v == run, "v: " << v << " run: " << run);
  }  
  BOOST_CHECK(po.iread<ureg>(bits_for_sentry) == magic);
}

BOOST_AUTO_TEST_CASE(g_clz_bitstrm_bigger){
  alloced_bref buf(4*c_register_bits);
  buf.zero();
  
  bref po(buf);

  // write two singular bit 0 @ at 1st and bit 1 at 3rd register 
  (buf + c_register_bits).iwrite(1,1);
  (buf + 3*c_register_bits + 1 ).iwrite(1,1);

  BOOST_CHECK(po.iclz() == c_register_bits);
  ++po;
  BOOST_CHECK(po.iclz() == 2*c_register_bits);
}

BOOST_AUTO_TEST_CASE(g_longer_lz_strings){
  const unsigned test_size = 5;
  alloced_bref buf(test_size*c_register_bits);
  buf.zero();
  bref c(buf);
  bref e(buf + test_size*c_register_bits);

  unsigned arbitrary_depth = (test_size-1)*c_register_bits + 1;
  (c + arbitrary_depth).iwrite(0xA5, 8); // 256 0's followed by  0b10100101 0{56}

  bref n(clz(c, e));
  BOOST_CHECK((c + arbitrary_depth) == n);
  n = clz(c = n + 1, e);
  BOOST_CHECK((c + 1) == n);
  n = clz(c = n + 1, e);
  BOOST_CHECK((c + 2) == n);
  n = clz(c = n + 1, e);
  BOOST_CHECK((c + 1) == n);
  n = clz(c = n + 1, e);
  BOOST_CHECK( e      == clz(c, e));
  alloced_bref buf2(2);
  buf2.write(1,2);  // b01
  BOOST_CHECK(buf2 + 1 == clz(buf2, buf2+2));
  buf2.write(2,2);  // b10
  BOOST_CHECK(buf2 + 0 == clz(buf2, buf2+2));
}

BOOST_AUTO_TEST_CASE(alloced_bref_copy){
  vector<reg> buf(505);
  const char  sentry = 42;
  buf.back() = sentry;
  
  bref p(&buf.front());
  bref po(p);

  // write a 10110011100011110000... pattern 4032 bits until when you
  // fall on an even 64 boundry
  int i = 0;
  do {
    i %= 64;
    ++i;
    p.iwrite(mask0(i), i);     // 1's
    p.iwrite(0, i);            // 0's
  } while((p - po) % 64 != 0 );
  
  // eliminate the 1's by copying the 2028 0's to the left
  // with successive copies
  i = 0;
  bref e = p;
  bref c = po;

  do {
    i %= 64;
    ++i;
        e = copy(c + i, e, c);  
        c += i;
  } while(e > c);

  // the result should be all zeros now!
  for(i = 0; i < 27; ++i){
    BOOST_CHECK(buf[i] == 0);
  }
  BOOST_CHECK(buf.back() == 42);  
}


BOOST_AUTO_TEST_CASE(g_example_0){
  alloced_bref alloc0(3);            // what can you do with 3 bits?
  bref p0 = alloc0;                  // make some 'pointers'
  bref p1 = p0;
  BOOST_CHECK(p0 == p1);             // pointers allign
  p0.iwrite(6, 3);                   // write small unsigned value 6, incrementing p0
  BOOST_CHECK(p0 > p1);              // p0 past p1
  BOOST_CHECK(p0 - p1 == 3);         // by 3 bits
  swap(p0, p1);                      // lets reorder our references
  BOOST_CHECK(p0.read<ureg>(3) == 6); // recover value
  alloced_bref alloc1(60);           // 20 times is actually the same because underneath its a 64 bit integer
  BOOST_CHECK(alloc0 != alloc1);     // don't know where alloc1 is wrt alloc0 but its not the same
  bref p2 = alloc1;
  bref p3 = copy(p0, p1, p2);        // copy the underlying bits 
  BOOST_CHECK(p3 - p2 == 3);         // (all three of them)
  BOOST_CHECK(p0.read<ureg>(3) == p2.read<ureg>(3)); // of course value got copied
  // three bits as signed can store {-4, -3, -2, -1, 0, 1, 2, 3}
  p0.write(-2, 3);                   // store -2 into alloc0
  // writing implicity gets sign correct, but for reading you must specifiy how to interpret underlying bits
  BOOST_CHECK(p0.read<reg>(3) == -2); 
  BOOST_CHECK(p0.read<reg>(3) == p2.read<reg>(3)); // what gives?! while -2 != 6, -2 == reg(6) when your register is 3-bit
  // back to the world of whole numbers, the subset of those of size must be 3 bits width, whats the maximum value you can store? 
  p0.write(7 /*or 0b111*/, 3);
  BOOST_CHECK(bref(p0).iread_rls<ureg>(3) == 14); // kind of strange, nothing to do with dance around temporary object
  p2.write(0 /*or 0b000*/, 3);       // here's a hint 
  BOOST_CHECK(bref(p2).iread_rls<ureg>(3) == 7);  // the minimum value is not zero but 7
  BOOST_CHECK(bref(p2).iread_rls<ureg>(0) == 0);  // note also zero takes *0* bits to store,  or {0}
  BOOST_CHECK(bref(p0).iread_rls<ureg>(2) == 6 && bref(p2).iread_rls<ureg>(2) == 3); // max respectivly at each bit size
  BOOST_CHECK(bref(p0).iread_rls<ureg>(1) == 2 && bref(p2).iread_rls<ureg>(1) == 1);
  // or {0}{1, 2}{3, 4, 5, 6}{7, 8, 9, 10, 11, 12, 13, 14}, where we have the ranges of 0, 1, 2, and 3 bits
}

BOOST_AUTO_TEST_CASE(g_documentation_example){
  const unsigned c_at_least_3_and_internally_stored_to_full_64_bit_boundry = 3;
  alloced_bref example_buf(c_at_least_3_and_internally_stored_to_full_64_bit_boundry);
  bref begin = example_buf;
  example_buf.iwrite(-4, min_bits(-4)); // write 3 bits encoding -4 to example_buf while advancing
  bref end = example_buf;                // now, encoded as a single signed integer, [begin, end) -> -4
  BOOST_CHECK(begin.read<reg>(end-begin) == -4);
}

BOOST_AUTO_TEST_CASE(g_popcount_simple){
  constexpr unsigned example_size = 1024;
  alloced_bref example(example_size);
  bref end = example + example_size;
  example.zero();
  example.write(1,1);
  BOOST_CHECK(popcount(example, end) == 1);
  (example+512).iwrite(1,1);
  BOOST_CHECK(popcount(example, end) == 2);
}

BOOST_AUTO_TEST_CASE(g_popcount_test){

  // maximally fill  elements one at a time and see that indeed it adds to the popcount
  constexpr unsigned example_size = 2057;
  alloced_bref example(example_size);
  example.zero();
  bref end = example + example_size;

  std::set<unsigned> pile;
  for(unsigned i = 0; i < example_size; ++i){
    ureg element = (i*1027)%example_size;
    pile.insert(element);
    (example + element).iwrite(1,1);
    BOOST_CHECK(popcount(example, end) == (i+1));
  }
  assert(pile.size() == example_size && "set should be completely covered");
}

BOOST_AUTO_TEST_CASE(g_rle_ureg){
 
  // unsigned 
  constexpr ureg max_packet = 14;
  constexpr ureg test_size  = 1024*1024/32;
  constexpr ureg total_bsize = 2*test_size*32;
  alloced_bref buf(total_bsize);
  
  for(unsigned packet_size = 2; packet_size <= max_packet; ++packet_size){
    bref c = buf;
    bref e = buf + total_bsize;
    
    for(ureg n = 0; n < test_size; ++n){
      ureg bsize = bref::bsize_rle(n, packet_size);
      bref cc = c;
      c.iwrite_rle(n, packet_size);
      if(ureg(c - cc) != bsize)
	BOOST_CHECK_MESSAGE(false, "mismatch: write_rle: " << (c - cc)
			    << " bsize_rle: " << bsize);
    }

    BOOST_CHECK( c < e );
    c = buf;

    for(ureg n = 0; n < test_size; ++n){
      bref cc = c;
      ureg restore = c.iread_rle<ureg>(packet_size);
      if(restore != n)
	BOOST_CHECK_MESSAGE(false, "restored value: " << restore << " does not match: "
			    << n);
      auto predicted = bref::bsize_rle<ureg>(n, packet_size);
      if(predicted != ureg(c - cc))
	BOOST_CHECK_MESSAGE(false, "predicted size: " << predicted
			    << " does not match actual size: " << ureg(c - cc) );
    }
    BOOST_TEST_MESSAGE("Using packet size " << packet_size << " series up to " << test_size
		       << " required " << (c - buf) << " bits.");
    
  }
}

BOOST_AUTO_TEST_CASE(g_bsize_rls_test){

  BOOST_CHECK(bref::bsize_rls<ureg>(0) == 0);   // b''
  BOOST_CHECK(bref::bsize_rls<ureg>(1) == 1);   // b'0'
  BOOST_CHECK(bref::bsize_rls<ureg>(2) == 1);   // b'1'
  BOOST_CHECK(bref::bsize_rls<ureg>(3) == 2);   // b'00'
  BOOST_CHECK(bref::bsize_rls<ureg>(4) == 2);   // b'01'
  BOOST_CHECK(bref::bsize_rls<ureg>(5) == 2);   // b'10'
  BOOST_CHECK(bref::bsize_rls<ureg>(6) == 2);   // b'11'
  BOOST_CHECK(bref::bsize_rls<ureg>(7) == 3);   // b'000'

  BOOST_CHECK(bref::bsize_rls<reg>(0) == 0);    // b''
  BOOST_CHECK(bref::bsize_rls<reg>(1) == 1);    // b'0'
  BOOST_CHECK(bref::bsize_rls<reg>(2) == 2);    // b'00'
  BOOST_CHECK(bref::bsize_rls<reg>(3) == 2);    // b'01'
  BOOST_CHECK(bref::bsize_rls<reg>(4) == 3);    // b'000'
  BOOST_CHECK(bref::bsize_rls<reg>(5) == 3);    // b'001'
  BOOST_CHECK(bref::bsize_rls<reg>(6) == 3);    // b'010'
  BOOST_CHECK(bref::bsize_rls<reg>(7) == 3);    // b'011'
  BOOST_CHECK(bref::bsize_rls<reg>(8) == 4);    // b'0000'

  BOOST_CHECK(bref::bsize_rls<reg>(-1) == 1);    // b'1'
  BOOST_CHECK(bref::bsize_rls<reg>(-2) == 2);    // b'11'
  BOOST_CHECK(bref::bsize_rls<reg>(-3) == 2);    // b'10'
  BOOST_CHECK(bref::bsize_rls<reg>(-4) == 3);    // b'111'
  BOOST_CHECK(bref::bsize_rls<reg>(-5) == 3);    // b'110'
  BOOST_CHECK(bref::bsize_rls<reg>(-6) == 3);    // b'101'
  BOOST_CHECK(bref::bsize_rls<reg>(-7) == 3);    // b'100'
  BOOST_CHECK(bref::bsize_rls<reg>(-8) == 4);    // b'1111'
  
}


// check_rle
//
// leading  ~ {-ve, 0, +ve} => {ignore, low, high}
// trailing ~ {-ve, 0, +ve} => {ignore, all_low, all_high}
template<class REG_UREG >
void
check_rle(REG_UREG value, unsigned bsize, unsigned packet_bsize,int leading, int trailing){

  alloced_bref buf(2*c_register_bits);
  unsigned payload = packet_bsize - 1;
  unsigned packets = std::max((bsize + (payload - 1) )/payload, 1U);
  unsigned restored_bsize = packets*payload;
   
  BOOST_CHECK(restored_bsize >= bsize and (restored_bsize - bsize) <= payload);

  bref c = buf;
  bref e = c;
  e.iwrite_rle<REG_UREG>(value, packet_bsize);
  BOOST_CHECK( (e - c) == packets*packet_bsize);
  REG_UREG restore = c.iread_rle<REG_UREG>(packet_bsize);
  BOOST_CHECK(restore == value);
  BOOST_CHECK(c == e);
  c = buf;
  if(leading >= 0)
    BOOST_CHECK( bool(restore >> (bsize-1))  == bool(leading));
  if(trailing == 0)
    BOOST_CHECK( (restore & mask0(bsize-1)) == 0);
  else if (trailing > 0)
    BOOST_CHECK( reg(restore & mask0(bsize-1)) == mask0(bsize-1));
}

template<class REG_UREG >
void
check_rls(REG_UREG value){
  alloced_bref buf(c_register_bits);
  bref c(buf);
  bref e(c);
  unsigned bsize = bref::bsize_rls<REG_UREG>(value);
  e.iwrite_rls(value, bsize);
  BOOST_CHECK((e - c) == bsize);
  REG_UREG restored = c.iread_rls<REG_UREG>(bsize);
  BOOST_CHECK(c == e);
  BOOST_CHECK(restored == value);
}

BOOST_AUTO_TEST_CASE(g_min_max_value_each_bsize){

  BOOST_CHECK(bref::bsize<ureg>(0)        == 0);
  BOOST_CHECK(bref::bsize<reg >(0)        == 0);
  BOOST_CHECK(bref::bsize_rle<ureg>(0, 5) == 5);
  BOOST_CHECK(bref::bsize_rle<reg >(0, 5) == 5);
  BOOST_CHECK(bref::bsize_rls<ureg>(0)    == 0);
  BOOST_CHECK(bref::bsize_rls<reg >(0)    == 0);
  
  BOOST_CHECK(sizeof(reg) == sizeof(ureg));
  
  // write the signed and unsigned version of min and max value and verify the bit
  // pattern matches the expected
  alloced_bref buf(2*c_register_bits);
  for(unsigned bsize = 1; bsize < unsigned(c_register_bits-1); ++bsize){

    bref c(buf);
    bref e(c);

    // unsigned
    BOOST_CHECK(bref::bsize<ureg>(bref::max<ureg>(bsize)) == bsize);
    e.iwrite(bref::max<ureg>(bsize), bsize);
    BOOST_CHECK((e - c) == bsize);
    BOOST_CHECK(c.iread<ureg>(bsize) == bref::max<ureg>(bsize));
    BOOST_CHECK(e == c);

    // signed (2's complement)
    // max<reg>(size = 1) or b0 is analogue to 0 (as with b,b00, b000 etc.),
    // otherwise bsize(max(x)) == x
    BOOST_CHECK(bsize == 1 or bref::max<reg>(bsize) != 0);
    BOOST_CHECK((bsize == 1 or (bref::bsize<reg>(bref::max<reg>(bsize)) == bsize)));
    BOOST_CHECK(bref::bsize<reg>(bref::min<reg>(bsize)) == bsize);
    // min value
    e = c = buf;
    e.iwrite(bref::min<reg>(bsize), bsize);
    BOOST_CHECK((e - c) == bsize);
    BOOST_CHECK(c.read<reg>(bsize) == bref::min<reg>(bsize));
    reg v = c.iread<reg>(bsize);
    BOOST_CHECK(e == c);
    BOOST_CHECK((reg(1) << (bsize-1)) & v);   // signed bit must be high
    BOOST_CHECK((v & mask0(bsize-1)) == 0);   // following low
    // max value
    e = c = buf;
    e.iwrite(bref::max<reg>(bsize), bsize);
    BOOST_CHECK((e - c) == bsize);
    BOOST_CHECK(c.read<reg>(bsize) == bref::max<reg>(bsize));
    v = c.iread<reg>(bsize);
    BOOST_CHECK(e == c);
    BOOST_CHECK(((reg(1) << (bsize -1)) & v) == 0);      // signed bit must be low
    BOOST_CHECK(bsize == 1 or (v == mask0(bsize-1)));    // following all high
    // rle - try all rle packet sizes for bsize
    BOOST_CHECK(bref::min<ureg>(bsize) == 0);
    for(unsigned packet = 2; false and packet < (c_register_bits - 2); ++packet){
      check_rle<ureg>(0, 0, packet, 0, 0);
      BOOST_CHECK(bref::min<ureg>(bsize) == 0);
      check_rle<ureg>(bref::max<ureg>(bsize), bsize, packet, 1, 1);
      BOOST_CHECK(bref::min<reg>(bsize) < 0);
      check_rle<reg >(bref::min<reg >(bsize), bsize, packet, 1, 0);
      check_rle<reg >(bref::max<reg >(bsize), bsize, packet, 0, 1); 
    }

    check_rls<ureg>(bref::min_rls<ureg>(bsize));
    check_rls<ureg>(bref::max_rls<ureg>(bsize));
    check_rls<reg >(bref::min_rls<reg >(bsize));
    check_rls<reg >(bref::max_rls<reg >(bsize));
  }
}

BOOST_AUTO_TEST_CASE(check_normalization){
  // by bouncing ramdomly then returning verify that the normalization function is working

  reg untouched;
  bref a(&untouched, 2);
  auto b = a;

  std::random_device rd; // obtain a random number from hardware
  std::mt19937 gen(rd()); // seed the generator
  std::normal_distribution d{0.0, 5e6};  // a wide distribution but also get some small numbers

  auto random_int = [&d, &gen]{ return std::lround(d(gen)); };

  auto m  = numeric_limits<reg>::max();
  auto M  = numeric_limits<reg>::min();
  auto am = m;

  auto record_extremum = [&m, &M, &am](reg x){
    m  = min(m, x);
    M  = max(M, x);
    if(std::labs(am) > std::labs(x))
      am = x;
    return;
  };
  
  BOOST_CHECK_MESSAGE(true, "quiet BOOST_CHECK message");
  for(ureg i = 0; i < 1000000; ++i){
    reg x = random_int();
    record_extremum(x);
    a += x;
    a -= x;

    if(a != b)
      BOOST_CHECK_MESSAGE(a != b,  "these two brefs are supposed to be equivalent a: "
			  << a << ", b: " << b << "\n");
    ;    
  }

  BOOST_TEST_MESSAGE("\nSampled Values: \nmin: " << m << " max: " << M << " closest to zero: " << am << "\n");
  
}


BOOST_AUTO_TEST_CASE(bref_equal){

  int const bufsz = 4032;
  alloced_bref p(bufsz);
  bref po(p);
  alloced_bref q(bufsz);
  bref qo(q);
  alloced_bref r(bufsz);
  bref ro(r);

  // write p,q as 10110011100011110000... pattern 4032 bits until falling on an even 64 boundry, r is a binary compliment of p
  int i = 0;
  do {
    i %= 64;
    ++i;
    
    p.iwrite(mask0(i), i);  // 1's
    q.iwrite(mask0(i), i);  // 1's
    p.iwrite(0, i);         // 0's
    q.iwrite(0, i);         // 0's

    r.iwrite(0, i);         // 0's
    r.iwrite(mask0(i), i);  // 1's

  } while((p - po) % 64 != 0 );

  BOOST_CHECK(r - ro == p - po);
  BOOST_CHECK(equal(po, po, qo));            // zero length
  BOOST_CHECK(equal(po, p, qo));             // full length
  BOOST_CHECK(equal(po + 42, p, qo + 42));   // arbitrary subset
  BOOST_CHECK(equal(po, p - 42, qo));        // arbitrary subset
  
  // for an irregular non zero incrementing sequence of i check
  // equivalence and non equivalence
  for(size_t i = 1; i < p - po; ){
    BOOST_CHECK( equal(po + i, p, qo + i));
    BOOST_CHECK(!equal(po + i, p, ro + i));
    int varies =   int(i*i) % 11;
    i += varies ? varies : 1;
  }
  for(int i = 1; i < 32; ++i){

    alloced_bref cpy(bufsz - 2*i);
    bref b = po + i;
    bref e = p  - i;
    bitstrm::copy(b, e, cpy);
    BOOST_CHECK(bitstrm::equal(b, e, cpy));
  }  
}

