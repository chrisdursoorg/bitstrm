// ref_unittest.cpp

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "BitStreamBref"

#include <boost/test/unit_test.hpp>
#include "bitstrm/alloced_bref.hpp"
#include "bitstrm/utility.hpp"
#include <limits>
#include <iostream>
#include <sstream>

using namespace boost::unit_test;
using namespace bitstrm;
using namespace std;

// print_human_bits
//
// formats bits as human readable string
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


BOOST_AUTO_TEST_CASE(min_size_write_and_restore)
{
  BOOST_TEST_MESSAGE("test min_bits() with bitstrm::ref");
  vector<char> buf(8);

  for(int i = 1024*1024; i > -1024*2024; --i){

    bref w(&buf.front());
    bref r(w);
    unsigned mbits = min_bits(i);

    // this many bits correctly stores the value
    w.write(mbits, mask0(mbits) & i);
    reg restore = r.read_reg(mbits);
    BOOST_CHECK(i == restore);
    // any less fails
    if(mbits){  // not zero!
      --mbits;
      w.write(mbits, mask0(mbits) & i);
      reg restore = r.read_reg(mbits); 
      BOOST_CHECK(i != restore);
    }
  } 
}



#ifdef USE_ENDIAN
// This test will only succeed with USE_ENDIAN on, otherwise the byte
// order as accessed a ureg and a char will be messed up
BOOST_AUTO_TEST_CASE(hello_world)
{
  char hw[] = "hello world";
  const int  sz = strlen(hw);

  bref p(hw);
  for(int i = 0; i < sz; ++i){
    BOOST_CHECK(p.iread_as<ureg>(CHAR_BIT) == (ureg)hw[i]);
  }
}
#endif

// This test of a simple codec to save space, for msg lower case chars
// and spaces, and we map space to 'z' + 1 in bottom 5 bits
BOOST_AUTO_TEST_CASE(hello_world2)
{
  const char   msg[] = "hello world";
  const int    sz = strlen(msg);
  vector<char> buf( bref::_chars(5*sz)); // round buf to full byte

  bref write(&buf.front());
  bref read(write);
  const char c_five_bits = (( 1<<5 ) - 1 );
  for_each( msg, msg + sz, [&write](const char v){ 
      if(v != ' ') 
        write.iwrite(5, v & c_five_bits );
      else 
        write.iwrite(5, ('z' + 1) & c_five_bits );});

  BOOST_CHECK((write - read) == 5 * sz); // its now compresed 8:5  or  .625 
  
  // check the decoded values
  for_each( msg, msg + sz, [&read](const char v){ 
      char decoded = read.iread_as<ureg>(5) | 0x60;
      if(decoded == 'z' + 1)
        decoded = ' ';
      BOOST_CHECK(decoded  == v );}   );
}

BOOST_AUTO_TEST_CASE(bitstrm_ops){  

  char buf[]="this data is irrelevant, its not going to be dereferenced or modified";
  bref p(buf, 16);

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

BOOST_AUTO_TEST_CASE(bitstrm_read_and_write){
  vector<char> buf(256);
  const char sentry = 42;
  buf.back() = sentry;
  
  bref p(&buf.front(),2);
  bref po(p);

  const int* bv = three_bit_signed_numbers;
  const int* ev = bv+sizeof(three_bit_signed_numbers)/sizeof(unsigned);    
  
  for (int bits = 4; bits <= 64; ++bits, p = po){
      
      for_each(bv, ev, [bits,&p](int v) { p.iwrite(bits, v);} );
      BOOST_CHECK(bref::_chars(p - po) <=  (buf.size() -1 ));
      for_each(bv, ev, [bits,&p](int v) { p.iwrite(bits, v);} );
      BOOST_CHECK(bref::_chars(p - po) <=  (buf.size() -1 ));
      p = po;
      for_each(bv, ev, [bits,&p](int v) { 
          reg restore(p.iread_as<reg>(bits));  
          BOOST_CHECK( restore == v);
        });
  }

  BOOST_CHECK(buf.back() == sentry);
}


BOOST_AUTO_TEST_CASE(bitstrm_read_write_arbitrary_depths){
  vector<char> buf(128);
  const char sentry = 42;
  buf.back() = sentry;

  bref p(&buf.front(),2);
  bref po(p);

  const int* bv = three_bit_signed_numbers;
  const int* ev = bv+sizeof(three_bit_signed_numbers)/sizeof(unsigned);    
   
  for (int bits = 4; bits <= 64; ++bits, p = po){

    // write
    for_each(bv, ev, [bits,&p](int v) { p.iwrite(bits, v);} );
    BOOST_CHECK(bits*(ev-bv) == (p-po));
    BOOST_CHECK(bref::_chars(p - po) <=  buf.size());
  
    // restore
    p = po;
    for_each(bv, ev, [bits,&p](int v) { 
        reg restore(p.iread_as<reg>(bits));  
        BOOST_CHECK(v == restore);
      } );
  }
  BOOST_CHECK(buf.back() == sentry);
}

BOOST_AUTO_TEST_CASE(ilzrun){
  ureg max_count = 11;
  ureg bits_for_test   = (max_count + 1)*(max_count + 2)/2;
  ureg magic           = 42;
  ureg bits_for_sentry = (min_bits(magic));
  alloced_bref buf( bits_for_test + bits_for_sentry);
  bref po(buf);

  for(int i = max_count + 1; i > 0 ; --i)
    buf.iwrite(i,1);  // write i-1 leading zeros
  buf.iwrite(bits_for_sentry, magic);
    
  for( int v = max_count; v != -1; --v){
    int run = po.ilzrun();
    BOOST_TEST_MESSAGE("any ... " << run);
    BOOST_CHECK_MESSAGE( v == run, "v: " << v << " run: " << run);
  }
  
  BOOST_CHECK(po.iread_ureg(bits_for_sentry) == magic);
}

BOOST_AUTO_TEST_CASE(alloced_bref_copy){
  vector<char> buf(505);
  const char sentry = 42;
  buf.back() = sentry;
  
  bref p(&buf.front());
  bref po(p);

  // write a 10110011100011110000... pattern 4032 bits until when you
  // fall on an even 64 boundry
  int i = 0;
  do {
    i %= 64;
    ++i;
    p.iwrite(i, mask0(i));     // 1's
    p.iwrite(i, 0);            // 0's
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

BOOST_AUTO_TEST_CASE(alloced_bref_equal){
  vector<char> buf(505);
  vector<char> buf2(505);
  vector<char> buf3(505);

  bref p(&buf.front());
  bref po(p);
  bref q(&buf2.front());
  bref qo(q);
  bref r(&buf3.front());
  bref ro(r);


  // write a 10110011100011110000... pattern 4032 bits until when you
  // fall on an even 64 boundry
  int i = 0;
  do {
    i %= 64;
    ++i;
    p.iwrite(i, mask0(i));  // 1's
    q.iwrite(i, mask0(i));  // 1's
    r.iwrite(i, 0);                 // 0's
    r.iwrite(i, mask0(i));  // 1's
    p.iwrite(i, 0);                 // 0's
    q.iwrite(i, 0);                 // 0's
  } while((p - po) % 64 != 0 );
  
  BOOST_CHECK(equal(po, po, qo));            // zero length
  BOOST_CHECK(equal(po, p, qo));             // full length
  BOOST_CHECK(equal(po + 42, p, qo + 42));   // arbitrary subset
  BOOST_CHECK(equal(po, p - 42, qo));        // arbitrary subset
  
  // for an irregular non zero incrementing sequence of i check
  // equivalence and non equivalence
  for(int i = 1; i < p - po; ){
    BOOST_CHECK( equal(po + i, p, qo + i));
    BOOST_CHECK(!equal(po + i, p, ro + i));
    int varies =   i*i % 11;
    i += varies ? varies : 1;
  }
}

BOOST_AUTO_TEST_CASE(read_write_rlp){
  constexpr unsigned  high_value = 31;
  alloced_bref buf(high_value*(c_register_bit_addr_sz + min_bits(high_value)));
  bref     beg = buf;
  for(unsigned i = 0; i < high_value; ++i ){
    buf.iwrite_rlp(i);
  }

  stringstream str;
  str << "Stored: " << high_value << " consecutive values in ";
  print_human_bits(str, ureg(buf - beg)) << " ";
  print_human_bits(str, double(buf - beg)/high_value) << " per value ";
  BOOST_TEST_MESSAGE(str.str());

  for(unsigned i = 0; i < high_value; ++i ){
    BOOST_CHECK(beg.iread_rlp() == i);
  }
  
}

BOOST_AUTO_TEST_CASE(read_write_rlp_with_tighter_addr){
  constexpr unsigned  high_value = 31;
  unsigned max_address = min_bits(high_value);
  alloced_bref buf(high_value*(max_address + min_bits(high_value)));
  bref     beg = buf;
  for(unsigned i = 0; i < high_value; ++i ){
    buf.iwrite_rlp(i, max_address);
  }

  stringstream str;
  str << "Stored: " << high_value << " consecutive values in ";
  print_human_bits(str, ureg(buf - beg)) << " ";
  print_human_bits(str, double(buf - beg)/high_value) << " per value ";
  BOOST_TEST_MESSAGE(str.str());;

  for(unsigned i = 0; i < high_value; ++i ){
    BOOST_CHECK(beg.iread_rlp(max_address) == i);
  }
}

BOOST_AUTO_TEST_CASE(read_write_rlp_bigger){
  constexpr unsigned  high_value = 123456;
  unsigned max_address = min_bits(high_value);
  alloced_bref buf(high_value*(max_address + min_bits(high_value)));
  bref     beg = buf;

  for(unsigned i = 0; i < high_value; ++i ){
    bref orig = buf;
    buf.iwrite_rlp(i, max_address);
    BOOST_CHECK(ureg(buf - orig) == bsize_rlp(i, max_address));
  }

  stringstream str;
  str << "Stored: " << high_value << " consecutive values in ";
  print_human_bits(str, ureg(buf - beg)) << " ";
  print_human_bits(str, double(buf - beg)/high_value) << " per value ";
  BOOST_TEST_MESSAGE(str.str());

  for(unsigned i = 0; i < high_value; ++i ){
    BOOST_CHECK(beg.iread_rlp(max_address) == i);
  }
}


BOOST_AUTO_TEST_CASE(example_0){
  alloced_bref alloc0(3);            // what can you do with 3 bits?
  bref p0 = alloc0;                  // make some 'pointers'
  bref p1 = p0;
  BOOST_CHECK(p0 == p1);             // pointers allign
  p0.iwrite(3, 6);                   // write small unsigned value 6, incrementing p0
  BOOST_CHECK(p0 > p1);              // p0 past p1
  BOOST_CHECK(p0 - p1 == 3);         // by 3 bits
  swap(p0, p1);                      // lets reorder our references
  BOOST_CHECK(p0.read_ureg(3) == 6); // recover value
  alloced_bref alloc1(60);           // 20 times is actually the same because underneath its a 64 bit integer
  BOOST_CHECK(alloc0 != alloc1);     // don't know where alloc1 is wrt alloc0 but its not the same
  bref p2 = alloc1;
  bref p3 = copy(p0, p1, p2);        // copy the underlying bits 
  BOOST_CHECK(p3 - p2 == 3);         // (all three of them)
  BOOST_CHECK(p0.read_ureg(3) == p2.read_ureg(3)); // of course value got copied
  // three bits as signed can store {-4, -3, -2, -1, 0, 1, 2, 3}
  p0.write(3, -2);                   // store -2 into alloc0
  BOOST_CHECK(p0.read_reg(3) == -2); // writing implicity gets sign correct, but for reading you must specifiy how to interpret underlying bits
  BOOST_CHECK(p0.read_reg(3) == p2.read_reg(3)); // what gives?! while -2 != 6, -2 == reg(6) when your register is 3-bit
  // back to the world of whole numbers, the subset of those of size must be 3 bits width, whats the maximum value you can store? 
  p0.write(3, 7 /*or 0b111*/);
  BOOST_CHECK(bref(p0).iread_rls(3) == 14); // kind of strange, nothing to do with dance around temporary object
  p2.write(3, 0 /*or 0b000*/);              // here's a hint 
  BOOST_CHECK(bref(p2).iread_rls(3) == 7);  // the minimum value is not zero but 7
  BOOST_CHECK(bref(p2).iread_rls(0) == 0);  // note also zero takes *0* bits to store,  or {0}
  BOOST_CHECK(bref(p0).iread_rls(2) == 6 && bref(p2).iread_rls(2) == 3); // max respectivly at each bit size
  BOOST_CHECK(bref(p0).iread_rls(1) == 2 && bref(p2).iread_rls(1) == 1);
  // or {0}{1, 2}{3, 4, 5, 6}{7, 8, 9, 10, 11, 12, 13, 14}, where we have the ranges of 0, 1, 2, and 3 bits
}


BOOST_AUTO_TEST_CASE(documentation_example){
  const unsigned c_at_least_3_and_internally_stored_to_full_64_bit_boundry = 3;
  alloced_bref example_buf(c_at_least_3_and_internally_stored_to_full_64_bit_boundry);
  bref begin = example_buf;
  example_buf.iwrite(min_bits(-4), -4); // write 3 bits encoding -4 to example_buf while advancing
  bref end = example_buf;               // now, encoded as a single signed integer, [begin, end) -> -4
  BOOST_CHECK(begin.read_reg(end-begin) == -4);
}

BOOST_AUTO_TEST_CASE(rlup_format){
  // run length unary preface works well with small numbers
 
  constexpr int top = 129;
  ureg bsize = 0;
  for(int i = 0; i < top; bsize += bsize_rlup(i++));
  alloced_bref buf(bsize);
  bref cur = buf;
  for(int i = 0; i < top; ++i)
    cur.iwrite_rlup(i);
  
  BOOST_CHECK(bsize == ureg(cur - buf));

  cur = buf;
  
  for(unsigned i = 0; i < top; ++i)
    BOOST_CHECK(cur.iread_rlup() == i);

  BOOST_CHECK(bsize = ureg(cur - buf));
}


BOOST_AUTO_TEST_CASE(rlp_v_rlup_format){
 
  // find the max values at each bitsize (at odd number sizes), specifically adding one bit to preface allows for 1 more bit on suffix
  stringstream out;
  alloced_bref buf(128); // allocate 'a whole lot' of bits
  for(int pref = 1; pref <= 32; ++pref){
    bref b = buf;
    b.iwrite(pref, 1);
    b.iwrite(pref-1, ureg(-1));
    bref d  = buf;
    out << "preface size: " << pref << " total bits: " << 2*pref -1 <<  " largest value: " << d.iread_rlup() << endl;
  }
  BOOST_TEST_MESSAGE(out.str());
  out.str("");
  out << "range where rulp is most effective is small, simple lz encoding is more effective for smaller (and tigher) valued distributions and rlp is better where the prefix size is bound" << endl;
  for(unsigned prefix_bits = 2; prefix_bits <= 6; ++prefix_bits){
    unsigned i = 2;
    while( (i  <=  (min_bits( 1 << prefix_bits) -1)) && (bsize_rlup(i) <= bsize_rlp(i, prefix_bits))){
      out << "prefix: " << prefix_bits << " i: " << i << " rlup: " << bsize_rlup(i) << " rlp: " << bsize_rlp(i, prefix_bits) << endl;
      i++;
      }

    out << "prefix bsize: " << prefix_bits;
    if (i  <=  (min_bits( 1 << prefix_bits) -1))
      out << " we run out of prefix bits" << endl;
    else
      out << " value where rlp is more efficient than rlup " << (i - 1) << endl;
  }
  BOOST_TEST_MESSAGE(out.str());
}


BOOST_AUTO_TEST_CASE(advance_test){

  constexpr unsigned example_size = 1234567;
  alloced_bref example(example_size);

  bref           end   = example + example_size;
  example.zero();
  for(int i = 0; i < 32; ++i )
    (example + 2*i).iwrite(1, 1);

  
  BOOST_CHECK(advance(example, 0) == example);
  BOOST_CHECK(advance(example, end, 0) == example);

  BOOST_CHECK(advance(example, 1) == (example + 2));
  BOOST_CHECK(advance(example, end, 1) == (example + 2));

  BOOST_CHECK(advance(example, 2) == (example + 4));
  BOOST_CHECK(advance(example, end, 2) == (example + 4));

  BOOST_CHECK(advance(example, end, 100) == end);

  for(int i = 0; i < 31; ++i){
    BOOST_CHECK(advance(example, i) == (example + i*2));
    BOOST_CHECK(advance(example, end, i) == (example + i*2));
  }

  BOOST_CHECK(advance(example, end, 32) == end);

  (example + (example_size - 2)).iwrite(1,1);
  BOOST_CHECK(advance(example, end, 32) != end);
  BOOST_CHECK(advance(example, end, 32) == (end-2));
  
}

BOOST_AUTO_TEST_CASE(popcount_simple){
  constexpr unsigned example_size = 1024;
  alloced_bref example(example_size);
  bref end = example + example_size;
  example.zero();
  example.write(1,1);
  BOOST_CHECK(popcount(example, end) == 1);
  (example+512).iwrite(1,1);
  BOOST_CHECK(popcount(example, end) == 2);
}

BOOST_AUTO_TEST_CASE(popcount_test){

  // maximally fill  elements one at a time and see that indeed it adds to the popcount
  constexpr unsigned example_size = 2057;
  alloced_bref example(example_size);
  example.zero();
  bref end = example + example_size;

  set<unsigned> pile;
  for(unsigned i = 0; i < example_size; ++i){
    ureg element = (i*1027)%example_size;
    pile.insert(element);
    (example + element).iwrite(1,1);
    BOOST_CHECK(popcount(example, end) == (i+1));
  }
  assert(pile.size() == example_size && "set should be completely covered");
}
