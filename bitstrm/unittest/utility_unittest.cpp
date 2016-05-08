// utility_unittest.cpp
//

#define BOOST_TEST_DYN_LINK

#define BOOST_TEST_MODULE "BitStreamUtility"

#include <boost/test/unit_test.hpp>
#include "bitstrm/utility.hpp"
#include <limits>
#include <array>

using namespace boost::unit_test;
using namespace bitint;
using namespace std;



// This should be another, albeit less performant, way to do bit_sign_adj
template<typename _T> inline ureg bit_sign_adj_check(_T v, boost::false_type)  { return v;}

template<typename _T> inline ureg bit_sign_adj_check(_T v, boost::true_type /*meansIsReallySigned*/){
  if( v < -1) 
    return ~v << 1;
  else if( v > -1)
    return v << 1;
  else
    return 1;
}
template<typename _T> inline ureg bit_sign_adj_check(_T v) { return bit_sign_adj_check(v, boost::is_signed<_T>()); }



template<typename V>
inline bool check_minmax(const V& a, const V& b){

  auto lhs = std::minmax(a, b);
  auto rhs = bitint::minmax(a, b);
  return lhs.first == rhs.first && lhs.second == rhs.second;
}


BOOST_AUTO_TEST_CASE(bitlib_min_max)
{

  const char one = 1;
  const char two = 2;

  BOOST_CHECK( std::min(one,two) == bitint::min(one, two));
  BOOST_CHECK( std::max(one,two) == bitint::max(one, two));
  BOOST_CHECK(check_minmax(one,two));

  const int neg_one = -1; 
  const int neg_two = -2;

  BOOST_CHECK( std::min(neg_one, neg_two) == bitint::min(neg_one, neg_two));
  BOOST_CHECK( std::max(neg_one, neg_two) == bitint::max(neg_one, neg_two));
  BOOST_CHECK(check_minmax(neg_one, neg_two));


  string hello("hello");
  string world("world");
  
  // for non primative integer, should just revert to std:: implementations
  BOOST_CHECK( std::min(hello, world) ==  bitint::min(hello, world));
  BOOST_CHECK( std::max(hello, world) ==  bitint::max(hello, world));
  BOOST_CHECK(check_minmax(hello,world));


  const bool trueValue  = true;
  const bool falseValue = false;
  BOOST_CHECK( std::min(trueValue, falseValue) == bitint::min(trueValue, falseValue));
  BOOST_CHECK( std::max(trueValue, falseValue) == bitint::max(trueValue, falseValue));
  BOOST_CHECK(check_minmax(trueValue, falseValue));
}



BOOST_AUTO_TEST_CASE(bit_sign_adj_test_static)
{
  BOOST_CHECK(min_bits(bit_sign_adj(0))  == 0);
  BOOST_CHECK(min_bits(bit_sign_adj(1))  == 2);
  BOOST_CHECK(min_bits(bit_sign_adj(2))  == 3);
  BOOST_CHECK(min_bits(bit_sign_adj(3))  == 3);
  BOOST_CHECK(min_bits(bit_sign_adj(4))  == 4);
  BOOST_CHECK(min_bits(bit_sign_adj(-1)) == 1);
  BOOST_CHECK(min_bits(bit_sign_adj(-2)) == 2);
  BOOST_CHECK(min_bits(bit_sign_adj(-3)) == 3);
  BOOST_CHECK(min_bits(bit_sign_adj(-4)) == 3);
}

BOOST_AUTO_TEST_CASE(bit_sign_adj_application_defect)
{

  BOOST_MESSAGE("The following sequence with the obviously incorrect result of min_bit()-> 5 revealed that I had mistakenly se the type 'acc' in my algorithm to reg!" );
  const std::array<reg, 16>  data = {{1, 0, 3, 1, 0, 4, 2, 4, 2, 2, 1, 1, 1, 3, 2, 0}};  

  ureg acc(0);
  ureg mb(0);
  for_each(data.begin(), data.end(), [&acc, &mb](const reg v){
      acc |= bit_sign_adj(v); 
      mb =  std::max(mb, min_bits(v));
      // BOOST_MESSAGE( "v: " << v << " bsa: " << bit_sign_adj(v) << " min bits bsa: " << min_bits(bit_sign_adj(v)) << " min_bits(v): " << min_bits(v));
      BOOST_CHECK(min_bits(bit_sign_adj(v)) == min_bits(v));
    }
    );

  BOOST_CHECK(min_bits(acc) == mb);
}


BOOST_AUTO_TEST_CASE(min_bits_signed_unsigned)
{
  {
    int signedNumbers[] = {0, 0, 3, 1, 0, 4, 3, 5, 3, 3, 2, 3};
    int* end = signedNumbers + sizeof(signedNumbers)/sizeof(int);
    
    BOOST_CHECK( min_bits(signedNumbers, end) == 4);
  }
  {
    unsigned unsignedNumbers[] = {0, 0, 3, 1, 0, 4, 3, 5, 3, 3, 2, 3};
    unsigned* end = unsignedNumbers + sizeof(unsignedNumbers)/sizeof(unsigned);
    
    BOOST_CHECK( min_bits(unsignedNumbers, end) == 3);
  }  
}

int  three_bit_signed_numbers[] = {3, 2, 1, 0, -1, -2, -3, -4};
int* three_bit_signed_numbers_e = three_bit_signed_numbers + sizeof(three_bit_signed_numbers)/sizeof(int);

BOOST_AUTO_TEST_CASE(signextend_3_bit_numbers)
{
  BOOST_TEST_MESSAGE("check signextend works on small integers");

  for_each(three_bit_signed_numbers, three_bit_signed_numbers_e, [](int v)
           { 
             int r = signextend<signed int,3>(v & 0x7);
             BOOST_CHECK(v  ==  r); 
           } );
}

BOOST_AUTO_TEST_CASE(min_bits_test_3_bit_unsigned_numbers)
{
  unsigned v[]                = { 0, 1, 2, 3, 4, 5, 6, 7};
  unsigned min_bits_2_store[] = { 0, 1, 2, 2, 3, 3, 3, 3};
  const unsigned* bv = v;
  const unsigned* ev = v+sizeof(v)/sizeof(unsigned);
  
  unsigned *r;
  BOOST_TEST_MESSAGE( "min_bits: (unsigned int) interface");        r = min_bits_2_store;  for_each(bv, ev, [&r](unsigned v) { BOOST_CHECK( min_bits(static_cast<unsigned int>(v)) == *r++); });
  BOOST_TEST_MESSAGE( "min_bits: (unsigned long) interface");       r = min_bits_2_store;  for_each(bv, ev, [&r](unsigned v) { BOOST_CHECK( min_bits(static_cast<unsigned long>(v)) == *r++); });
  BOOST_TEST_MESSAGE( "min_bits: (unsigned long long ) interface"); r = min_bits_2_store;  for_each(bv, ev, [&r](unsigned v) { BOOST_CHECK( min_bits(static_cast<unsigned long long>(v)) == *r++); });
  BOOST_TEST_MESSAGE( "min_bits:  iterator interface"); BOOST_CHECK(min_bits(bv, ev) == 3);
}

BOOST_AUTO_TEST_CASE(min_bits_test_3_bit_signed_numbers)
{
  unsigned min_bits_2_store[] = { 3, 3, 2, 0,  1,  2,  3,  3}; 

  const int* bv = three_bit_signed_numbers;
  const int* ev = three_bit_signed_numbers + sizeof(three_bit_signed_numbers)/sizeof(unsigned);
    
  unsigned* r;
  BOOST_TEST_MESSAGE( "(int) interface");        r = min_bits_2_store;  for_each(bv, ev, [&r](int v) { BOOST_CHECK_MESSAGE( min_bits(static_cast<int>(v)) == *r, *r << " not " << min_bits(v)) ; r++;});
  BOOST_TEST_MESSAGE( "(long) interface");       r = min_bits_2_store;  for_each(bv, ev, [&r](int v) { BOOST_CHECK_MESSAGE( min_bits(static_cast<long>(v)) == *r, *r << " not " << min_bits(v)); r++;});
  BOOST_TEST_MESSAGE( "(long long ) interface"); r = min_bits_2_store;  for_each(bv, ev, [&r](int v) { BOOST_CHECK_MESSAGE( min_bits(static_cast<long long>(v)) == *r, *r << " not " << min_bits(v)); r++; });
  BOOST_TEST_MESSAGE( "*iterator interface (int)*");  BOOST_CHECK(min_bits(bv, ev) == 3);
}


BOOST_AUTO_TEST_CASE(min_bits_64_million_checks){

  // construct a buffer of 512 size repeating the numbers {3,2,1,0,-1,-2,-3,-4}
  int buf [512];
  int* c = buf;
  while( c < buf + sizeof(buf)/sizeof(int)) { c = std::copy(three_bit_signed_numbers, three_bit_signed_numbers+sizeof(three_bit_signed_numbers)/sizeof(int), c); }
  
  // count the total min_bits size if 512 block were repeated 1 M times
  int* e = c;
  long totalSize(0);
  for(int i = 0; i < 1024*128; ++i )
    totalSize += min_bits(buf,e) * 512;

  BOOST_CHECK(totalSize == 1024*128*512*3);
}


BOOST_AUTO_TEST_CASE(min_bits_64_million_slower_way){

  // this time we aggegate the same number of bits but we use less optimum branching approach directly on original {3,2,1,0,-1,-2,-3,-4} sequence
  int* b = three_bit_signed_numbers;
  int* e = b + sizeof(three_bit_signed_numbers)/sizeof(int);

  long totalSize(0);

  for(int i = 0; i < 1024*128*64; ++i ){
    ureg mb = 0;
    for(const int* c = b; c != e; ++c)
      mb = std::max(mb, min_bits(*c)); 
    totalSize += mb * (e-b);
  }

  BOOST_CHECK(totalSize == 1024*128*512*3);
}



