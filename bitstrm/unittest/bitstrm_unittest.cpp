// bitstrm_test.cpp

#define BOOST_TEST_DYN_LINK

#define BOOST_TEST_MODULE "BitStreamBitStream"

#include <boost/test/unit_test.hpp>
#include "bitstrm/bitstrm.hpp"
#include "bitstrm/utility.hpp"
#include <limits>

using namespace boost::unit_test;
using namespace bitint;
using namespace std;


BOOST_AUTO_TEST_CASE(min_size_write_and_restore)
{

  BOOST_MESSAGE("test min_bits() with bitstrms");
  vector<char> buf(8);

  for(int i = 1024*1024; i > -1024*2024; --i){

    bitstrm w(&buf.front());
    bitstrm r(w);
    unsigned mbits = min_bits(i);

    // this many bits correctly stores the value
    w.write(mbits, bitstrm::mask(mbits) & i);
    reg restore = r.read_reg(mbits); 
    BOOST_CHECK(i == restore);

    // any less fails
    if(mbits){  // not zero!
      --mbits;
      w.write(mbits, bitstrm::mask(mbits) & i);
      reg restore = r.read_reg(mbits); 
      BOOST_CHECK(i != restore);
    }
  } 
}


#ifdef USE_ENDIAN
// This test will only succeed with USE_ENDIAN on, otherwise the byte order as accessed a ureg and a char
// will be messed up
BOOST_AUTO_TEST_CASE(hello_world)
{
  char hw[] = "hello world";
  const int  sz = strlen(hw);

  bitstrm p(hw);
  for(int i = 0; i < sz; ++i){
    BOOST_CHECK(p.iread_as<ureg>(CHAR_BIT) == (ureg)hw[i]);
  }
}
#endif


// This test of a simple codec to save space, for msg lower case chars and spaces, and we map space to 'z'+1
// in bottom 5 bits
BOOST_AUTO_TEST_CASE(hello_world2)
{
  const char msg[] = "hello world";
  const int  sz = strlen(msg);
  vector<char> buf( bitstrm::chars(5*sz)); // round buf to full byte

  bitstrm write(&buf.front());
  bitstrm read(write);
  const char c_five_bits = (( 1<<5)-1);
  for_each( msg, msg + sz, [&write](const char v){ 
      if(v != ' ') 
        write.iwrite(5, v & c_five_bits );
      else 
        write.iwrite(5, ('z' + 1) & c_five_bits );});

  BOOST_CHECK((write - read) == 5 * sz); // its now compresed 8:5  or  .625 
  
  // check the decoded values
  for_each( msg, msg + sz, [&read](const char v){ 
      char decoded = read.iread_as<ureg>(5) | 0x60;
      if(decoded == 'z'+1)
        decoded = ' ';
      BOOST_CHECK(decoded  == v );}   );
}


BOOST_AUTO_TEST_CASE(bitstrm_ops){  
  char buf[]="this data is irrelevant, its not going to be dereferenced or modified";
  bitstrm p(buf, 16);

  bitstrm o = p++;
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
int* three_bit_signed_numbers_e = three_bit_signed_numbers + sizeof(three_bit_signed_numbers)/sizeof(int);

BOOST_AUTO_TEST_CASE(bitstrm_read_and_write){
  vector<char> buf(256);
  const char sentry = 42;
  buf.back() = sentry;
  
  bitstrm p(&buf.front(),2);
  bitstrm po(p);

  const int* bv = three_bit_signed_numbers;
  const int* ev = bv+sizeof(three_bit_signed_numbers)/sizeof(unsigned);    
  
  for (int bits = 4; bits <= 64; ++bits, p = po){
      
      for_each(bv, ev, [bits,&p](int v) { p.iwrite(bits, v);} );
      BOOST_CHECK(bitstrm::chars(p - po) <=  (buf.size() -1 ));
      for_each(bv, ev, [bits,&p](int v) { p.iwrite(bits, v);} );
      BOOST_CHECK(bitstrm::chars(p - po) <=  (buf.size() -1 ));
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

  bitstrm p(&buf.front(),2);
  bitstrm po(p);

  const int* bv = three_bit_signed_numbers;
  const int* ev = bv+sizeof(three_bit_signed_numbers)/sizeof(unsigned);    
   
  for (int bits = 4; bits <= 64; ++bits, p = po){

    // write
    for_each(bv, ev, [bits,&p](int v) { p.iwrite(bits, v);} );
    BOOST_CHECK(bits*(ev-bv) == (p-po));
    BOOST_CHECK(bitstrm::chars(p - po) <=  buf.size());
  
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
    vector<char> buf(101);
    const char sentry = 42;
    buf.back() = sentry;

    bitstrm p(&buf.front());
    bitstrm po(p);

    for(int i = 11; i > 0 ; --i){
      p.iwrite(i,1);  // write i-1 leading zeros
    }

    p = po;
    for( int v = 10; v != -1; --v){
      BOOST_CHECK( v ==  (int)(p.ilzrun()));
    }
    BOOST_CHECK(buf.back() == sentry);
}


BOOST_AUTO_TEST_CASE(bitstrm_copy){
  vector<char> buf(505);
  const char sentry = 42;
  buf.back() = sentry;
  
  bitstrm p(&buf.front());
  bitstrm po(p);

  // write a 10110011100011110000... pattern 4032 bits until when you fall on an even 64 boundry
  int i = 0;
  do {
    i %= 64;
    ++i;
    p.iwrite(i, bitstrm::mask(i));  // 1's
    p.iwrite(i, 0);                 // 0's
  } while((p - po) % 64 != 0 );
  
  // eliminate the 1's by copying the 2028 0's to the left
  // with successive copies
  i = 0;
  bitstrm e = p;
  bitstrm c = po;

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

BOOST_AUTO_TEST_CASE(bitstrm_equal){
  vector<char> buf(505);
  vector<char> buf2(505);
  vector<char> buf3(505);

  bitstrm p(&buf.front());
  bitstrm po(p);
  bitstrm q(&buf2.front());
  bitstrm qo(q);
  bitstrm r(&buf3.front());
  bitstrm ro(r);


  // write a 10110011100011110000... pattern 4032 bits until when you fall on an even 64 boundry
  int i = 0;
  do {
    i %= 64;
    ++i;
    p.iwrite(i, bitstrm::mask(i));  // 1's
    q.iwrite(i, bitstrm::mask(i));  // 1's
    r.iwrite(i, 0);                 // 0's
    r.iwrite(i, bitstrm::mask(i));  // 1's
    p.iwrite(i, 0);                 // 0's
    q.iwrite(i, 0);                 // 0's
  } while((p - po) % 64 != 0 );
  
  BOOST_CHECK(equal(po, po, qo));            // zero length
  BOOST_CHECK(equal(po, p, qo));             // full length
  BOOST_CHECK(equal(po + 42, p, qo + 42));   // arbitrary subset
  BOOST_CHECK(equal(po, p - 42, qo));        // arbitrary subset
  
  // for an irregular non zero incrementing sequence of i check equivalence and non equivalence
  for(int i = 1; i < p - po; ){
    BOOST_CHECK( equal(po + i, p, qo + i));
    BOOST_CHECK(!equal(po + i, p, ro + i));
    int varies =   i*i % 11;
    i += varies ? varies : 1;
  }
}