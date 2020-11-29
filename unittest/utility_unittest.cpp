// utility_unittest.cpp
//

#define BOOST_TEST_DYN_LINK

#define BOOST_TEST_MODULE "BitStreamUtility"

#include <boost/test/unit_test.hpp>
#include "bitstrm/alloced_bref.hpp"
#include "bitstrm/utility.hpp"
#include <limits>
#include <array>
#include <iostream>
#include <iomanip>

using namespace boost::unit_test;
using namespace bitstrm;
using namespace std;

// This is another, albeit less performant, way to do bit_sign_adj
template<typename _T>
ureg
bit_sign_adj_check(_T v, std::false_type)  {
  return v;
}

template<typename _T>
ureg
bit_sign_adj_check(_T v, std::true_type /*meansIsReallySigned*/){
  if( v < -1) 
    return ~v << 1;
  else if( v > -1)
    return v << 1;
  else
    return 1;
}

template<typename _T>
ureg
bit_sign_adj_check(_T v) {
  return bit_sign_adj_check(v, std::is_signed<_T>());
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

  BOOST_TEST_MESSAGE("The following sequence revealed an earlier algorithm error" );
  const std::array<reg, 16>  data = {{1, 0, 3, 1, 0, 4, 2, 4, 2, 2, 1, 1, 1, 3, 2, 0}};  

  ureg acc(0);
  ureg mb(0);
  for_each(data.begin(), data.end(), [&acc, &mb](const reg v){
      acc |= bit_sign_adj(v); 
      mb =  std::max(mb, min_bits(v));
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

  for_each(three_bit_signed_numbers, std::end(three_bit_signed_numbers), [](int v)
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

BOOST_AUTO_TEST_CASE(bit_numeric_limits){
  
  BOOST_TEST_MESSAGE("some magnitudes turned into minimum and maximum numbers");
  BOOST_CHECK(numeric_limits_signed_min  (0) == 0);
  BOOST_CHECK(numeric_limits_signed_max  (0) == 0);
  BOOST_CHECK(numeric_limits_unsigned_min(0) == 0);
  BOOST_CHECK(numeric_limits_unsigned_max(0) == 0);  

  BOOST_CHECK(numeric_limits_signed_min  (1) == -1);
  BOOST_CHECK(numeric_limits_signed_max  (1) ==  0);
  BOOST_CHECK(numeric_limits_unsigned_min(1) ==  0);
  BOOST_CHECK(numeric_limits_unsigned_max(1) ==  1);  

  BOOST_CHECK(numeric_limits_signed_min  (2) == -2);
  BOOST_CHECK(numeric_limits_signed_max  (2) ==  1);
  BOOST_CHECK(numeric_limits_unsigned_min(2) ==  0);
  BOOST_CHECK(numeric_limits_unsigned_max(2) ==  3);  
  

  BOOST_CHECK(numeric_limits_signed_min  (3) == -4);
  BOOST_CHECK(numeric_limits_signed_max  (3) ==  3);
  BOOST_CHECK(numeric_limits_unsigned_min(3) ==  0);
  BOOST_CHECK(numeric_limits_unsigned_max(3) ==  7);  
  
  
  BOOST_CHECK(numeric_limits_signed_min  (63) == -4611686018427387904);
  BOOST_CHECK(numeric_limits_signed_max  (63) ==  4611686018427387903);
  BOOST_CHECK(numeric_limits_unsigned_min(63) ==  0);
  BOOST_CHECK(numeric_limits_unsigned_max(63) ==  9223372036854775807);

  BOOST_CHECK(numeric_limits_signed_min  (64) ==  LLONG_MIN);
  BOOST_CHECK(numeric_limits_signed_max  (64) ==  LLONG_MAX);
  BOOST_CHECK(numeric_limits_unsigned_min(64) ==  0);
  BOOST_CHECK(numeric_limits_unsigned_max(64) ==  ULLONG_MAX);
}



