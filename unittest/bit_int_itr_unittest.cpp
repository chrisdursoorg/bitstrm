// bit_int_itr_unittest.cpp
//

#define BOOST_TEST_DYN_LINK

#define BOOST_TEST_MODULE "BitIntIter"

#include <boost/test/unit_test.hpp>
#include "bitstrm/alloced_bref.hpp"
#include "bitstrm/reg.hpp"
#include "bitstrm/bref.hpp"
#include "bitstrm/utility.hpp"
#include "bitstrm/bit_int_itr.hpp"
#include <limits>
#include <algorithm>

using namespace boost::unit_test;
using namespace bitstrm;
using namespace std;



BOOST_AUTO_TEST_CASE( store_values_two_different_ways )
{
  // initializing arrays
  std::vector<int> container{{1,2,3,4,-4,5,6,7,192,323,3223}};
  unsigned bitSz = min_bits(container.begin(),container.end());
  BOOST_CHECK_MESSAGE(bitSz == 13, "for the initValues, 13 bits will hold all values in reg format");
  unsigned numbers_bsize = bitSz*container.size();
  const char sentry = 42;                   // set "magic number" to see that neither front nor back is overwritten
  alloced_bref buf (numbers_bsize + 2*8);
  alloced_bref buf2(numbers_bsize + 2*8);
  buf .iwrite(8, sentry);                   // mark the ends for later;
  buf2.iwrite(8, sentry);
  (buf  +  numbers_bsize).write(8, sentry);
  (buf2 +  numbers_bsize).write(8, sentry);
  

  bit_int_itr<13,reg> bc(buf2);
  bit_int_itr<13,reg> be(buf2+numbers_bsize);
  BOOST_TEST_MESSAGE("mechanism 1 to code container values into bitstrm backed by buf");
  for_each(container.begin(), container.end(), [bitSz,&buf](int v) { buf.iwrite(bitSz, v); });
  BOOST_TEST_MESSAGE("mechanism 2 to code container values into bitstrm backed by buf2");
  BOOST_CHECK(be == copy(container.begin(), container.end(), bc));
  buf .reset();
  buf2.reset();
  BOOST_CHECK(buf.iread_reg(8) == sentry && buf2.iread_reg(8) == sentry);
  BOOST_CHECK((buf  + numbers_bsize).iread_reg(8) == sentry);
  BOOST_CHECK((buf2 + numbers_bsize).iread_reg(8) == sentry);
  BOOST_TEST_MESSAGE("lots of different ways to be equal");
  buf .reset();
  buf2.reset();
  BOOST_CHECK_MESSAGE(equal(buf, buf + 2*8 + numbers_bsize, buf2), "the bitstrms should be equal");
  BOOST_CHECK_MESSAGE(equal(bc, be, container.begin()), "the itr should return the original sequence");
}

BOOST_AUTO_TEST_CASE(boost_style_itr)
{
  alloced_bref any(0);
  const int arbitrary_size = 4;
  typedef boost_facade_style_itr<ureg, arbitrary_size> test_itr;
  test_itr begin(any);
  test_itr empty_end(any);
  test_itr two_end(any + arbitrary_size*2 );
  
  // address checking
  BOOST_CHECK( begin == empty_end);
  BOOST_CHECK( begin < two_end );
  BOOST_CHECK( begin + 1 < two_end );
  BOOST_CHECK( begin + 2 == two_end );
  BOOST_CHECK( begin != two_end );
  BOOST_CHECK( two_end - begin == 2);
  test_itr t = begin  + 1;
  BOOST_CHECK( --t == begin);
  BOOST_CHECK( t + 2 == two_end );
  BOOST_CHECK( t     == begin);
  BOOST_CHECK( t++   == begin);
  BOOST_CHECK( --t   == begin);
  BOOST_CHECK( t++   == begin);
  BOOST_CHECK( ++t   == two_end);

  // assignments to some space on the stack
  alloced_bref buf(arbitrary_size*5);
  any = buf;
  begin   = test_itr(any);
  test_itr cur(begin);
  test_itr end(any + arbitrary_size*5);
  for(int i = 0; cur != end; ++cur, ++i){
    *cur = i;
    BOOST_CHECK(cur - begin == i);
  }
  
  // read it
  cur = begin;
  for(unsigned i = 0; cur != end; ++cur, ++i)
    BOOST_CHECK(*cur == i);
  
}

BOOST_AUTO_TEST_CASE( store_values_two_different_ways_boost_style )
{
  // initializing arrays
  int initValues[] = {1,2,3,4,4,5,6,7,7,7,7};
  typedef  std::vector<int> containerType;
  containerType container(initValues, initValues + sizeof(initValues)/sizeof(int));

  const unsigned bitSz = 4;
  typedef boost_facade_style_itr<ureg, bitSz> test_itr;

  BOOST_CHECK(min_bits(container.begin(), container.end()) <= bitSz);
  std::vector<char> buf(bref::_chars(bitSz * container.size()) + 2);
  const char sentry = 42;       // set "magic number" to see that neither front() or back() is overwritten
  buf.front() = sentry;
  buf.back() = sentry;
  std::vector<char> buf2(buf);
  bref p((&buf.front()+1));            // bitstream starts after "magic number"
  bref p2(&buf2.front()+1);
  bref pe(p2 + container.size()*bitSz); // and continues until "magic number"

  test_itr bc(p2);
  test_itr be(pe);
  BOOST_TEST_MESSAGE("mechanism 1 to code container values into bitstrm backed by buf");
  for_each(container.begin(), container.end(), [bitSz,&p](containerType::value_type value) { p.iwrite(bitSz, value); });
  BOOST_TEST_MESSAGE("mechanism 2 to code container values into bitstrm backed by buf2");
  BOOST_CHECK(be == copy(container.begin(), container.end(), bc));
  BOOST_CHECK(buf.front() == sentry && buf2.front() == sentry);
  BOOST_CHECK(buf.back() == sentry  && buf2.back() == sentry);
  BOOST_TEST_MESSAGE("lots of different ways to be equal");
  BOOST_CHECK_MESSAGE(equal(buf.begin(), buf.end(), buf2.begin()), "the memory should be equal");
  BOOST_CHECK_MESSAGE(equal(p2, pe, bref(&buf2.front()+1)), "the bitstrms should be equal");
  BOOST_CHECK_MESSAGE(equal(bc, be, container.begin()), "buf should return the original data");
}

BOOST_AUTO_TEST_CASE( store_values_two_different_with_dynamic_bsize_ways_boost_style )
{
  // initializing arrays
  int initValues[] = {1,2,3,4,4,5,6,7,7,7,7};
  typedef  std::vector<int> containerType;
  containerType container(initValues, initValues + sizeof(initValues)/sizeof(int));

  unsigned bitSz = 4;
  typedef boost_facade_style_itr<ureg> test_itr;

  BOOST_CHECK(min_bits(container.begin(), container.end()) <= bitSz);
  std::vector<char> buf(bref::_chars(bitSz * container.size()) + 2);
  const char sentry = 42;       // set "magic number" to see that neither front() or back() is overwritten
  buf.front() = sentry;
  buf.back() = sentry;
  std::vector<char> buf2(buf);
  bref p((&buf.front()+1));            // bitstream starts after "magic number"
  bref p2(&buf2.front()+1);
  bref pe(p2 + container.size()*bitSz); // and continues until "magic number"

  test_itr bc(p2, bitSz*1);
  test_itr be(pe, bitSz*1);
  BOOST_TEST_MESSAGE("mechanism 1 to code container values into bitstrm backed by buf");
  for_each(container.begin(), container.end(), [bitSz,&p](containerType::value_type value) { p.iwrite(bitSz, value); });
  BOOST_TEST_MESSAGE("mechanism 2 to code container values into bitstrm backed by buf2");
  BOOST_CHECK(be == copy(container.begin(), container.end(), bc));
  BOOST_CHECK(buf.front() == sentry && buf2.front() == sentry);
  BOOST_CHECK(buf.back() == sentry  && buf2.back() == sentry);
  BOOST_TEST_MESSAGE("lots of different ways to be equal");
  BOOST_CHECK_MESSAGE(equal(buf.begin(), buf.end(), buf2.begin()), "the memory should be equal");
  BOOST_CHECK_MESSAGE(equal(p2, pe, bref(&buf2.front()+1)), "the bitstrms should be equal");
  BOOST_CHECK_MESSAGE(equal(bc, be, container.begin()), "buf should return the original data");
}


BOOST_AUTO_TEST_CASE( sort_values )
{
  // initializing arrays
  std::vector<int> container{{1,2,3,4,-4,5,6,7,192,323,3223}};
  unsigned bitSz = min_bits(container.begin(),container.end());
  BOOST_CHECK_MESSAGE(bitSz == 13, "for the container, 13 bits will hold all values in reg format");
  unsigned numbers_bsize = bitSz*container.size();
  alloced_bref buf(numbers_bsize + 2*8 + 64); // keep some extra space for 8 bit padding
  const char sentry = 42;                // "magic number" to see that front and back are preserved
  buf.write(8,sentry);
  (buf + 8 + numbers_bsize).write(8, sentry);
  bref p(buf + 8); 
  bref p0(p);                            // bitstream to start after "magic number"
  bref pe(p0 + numbers_bsize);           // and continues until back "magic number"

  bit_int_itr<13,reg> bc(p0);
  bit_int_itr<13,reg> be(pe);
  
  for_each(container.begin(), container.end(), [bitSz,&p](int value) { p.iwrite(bitSz, value); });
  stringstream str;
  str << "pre-sort: "; for_each(bc, be, [&str](reg v ){ str << v << ", ";});   str << endl;
  BOOST_TEST_MESSAGE(str.str());
  BOOST_CHECK(equal(container.begin(),container.end(), bc));
  sort(bc, be);
  BOOST_CHECK(buf.read_reg(8) == sentry);
  BOOST_CHECK(pe .read_reg(8) == sentry);
  str.str("");
  str << "pst-sort: "; for_each(bc, be, [&str](reg v ){ str << v << ", ";});   str << endl;
  BOOST_TEST_MESSAGE(str.str());
  sort(container.begin(), container.end());
  BOOST_CHECK(equal(container.begin(),container.end(), bc));
}

BOOST_AUTO_TEST_CASE(bit_int_itr_ops)
{
  // initializing arrays
  typedef  std::vector<int> containerType;
  int initValues[] = {1,2,3,4,-4,5,6,7,192,323,3223};
  containerType container(initValues, initValues + sizeof(initValues)/sizeof(int));
  unsigned bitSz = min_bits(container.begin(),container.end());
  BOOST_CHECK(bitSz == 13);
  std::vector<char> buf(bref::_chars(bitSz*container.size()) +2);
  const char sentry = 42;
  buf.front() = sentry;
  buf.back() = sentry;
  bref p((&buf.front()+1));
  bref p0(p);
  bref pe(p + container.size()*bitSz);

  for_each(container.begin(), container.end(), [bitSz,&p](containerType::value_type value) { p.iwrite(bitSz, value); });
  BOOST_CHECK(p == pe);
  BOOST_CHECK(buf.front() == sentry);
  BOOST_CHECK(buf.back() == sentry);
  BOOST_CHECK((bitSz*container.size()) == (ureg)(p - p0));

  // use std and bit_int_itr interators to check values
  containerType::const_iterator b = container.begin();
  containerType::const_iterator e = container.end();
  p = p0;
  assert( bitSz == 13 && "We have a mix of lvalue and dynamic, fine as long as they remain the same"); 
  bit_int_itr<13,reg> verify(p);
  for(; b != e; ++b, ++verify)
    BOOST_CHECK(*b == *verify);

  bit_int_itr<13,reg> bc(p0);
  bit_int_itr<13,reg> be(pe);

  BOOST_CHECK(*bc == 1);
  BOOST_CHECK(*(bc+5) == 5); 
  BOOST_CHECK((*(bc+5) = 32) == 32);
  BOOST_CHECK(*(bc + 5) == 32);
  BOOST_CHECK(*(++bc) == 2);
  BOOST_CHECK(*(--bc) == 1);
  BOOST_CHECK(*(bc++) == 1);
  BOOST_CHECK(*(bc--) == 2);
  BOOST_CHECK(*bc == 1);

  BOOST_CHECK(*(be-2) == 323);
  BOOST_CHECK(*(be-1) == 3223);
  swap(*(be-1), *(be-2));
  BOOST_CHECK(buf.front() == sentry);
  BOOST_CHECK(buf.back() == sentry);
  BOOST_CHECK(*(be-1) == 323);
  BOOST_CHECK(*(be-2) == 3223);
  swap(*(be-1), *(be-2)); // resore to original order/values
  BOOST_CHECK(buf.front() == sentry);
  BOOST_CHECK(buf.back() == sentry);
  bc[5] = 5; 

  BOOST_CHECK(equal(container.begin(),container.end(), bc));
  stringstream str;
  str << "pre-swap: ";  for_each(bc, be, [&str](reg v ){ str << v << ", ";});  str << endl;
  BOOST_TEST_MESSAGE(str.str());
  swap(*(bc+0), *(bc+4));
  str.str("");
  str << "swap 0,4: ";  for_each(bc, be, [&str](reg v ){ str << v << ", ";});   str << endl;
  BOOST_TEST_MESSAGE(str.str());
  swap(*(bc+0), *(bc+4));
  str.str("");
  str << "swap 0,4: ";  for_each(bc, be, [&str](reg v ){ str << v << ", ";});   str << endl;
  BOOST_TEST_MESSAGE(str.str());
  BOOST_CHECK(buf.front() == sentry);
  BOOST_CHECK(buf.back() == sentry);
  sort(bc, be);
  BOOST_CHECK(buf.front() == sentry);
  BOOST_CHECK(buf.back() == sentry);
  str.str("");
  str << "pst-sort: ";  for_each(bc, be, [&str](reg v ){ str << v << ", ";});   str << endl;
  BOOST_TEST_MESSAGE(str.str());
  sort(container.begin(), container.end());
  BOOST_CHECK(equal(container.begin(),container.end(), bc));
}


BOOST_AUTO_TEST_CASE(sort_10_bit_unsigned)
{
  const size_t testSize = 512*1024; 
  BOOST_TEST_MESSAGE( "ureg v 10 bits sort with " << testSize << " elements");
  vector<ureg> testSet(testSize);  
  vector<ureg>::iterator c(testSet.begin());
  vector<ureg>::iterator e(testSet.end());
  const ureg ten_bit_mask((1<<10) -1);
  vector<char> buf((testSize *10 + 7)/8);

  bit_int_itr<10,ureg>  b0(bref(&buf.front()));
  bit_int_itr<10,ureg>  bc(b0);

  for(; c < e; ++c, ++bc)
    *bc = *c = (ureg)rand() & ten_bit_mask;
    
  bit_int_itr<10,ureg> be(bc);
  bc = b0;
        
  BOOST_TEST_MESSAGE("sort ureg");
  sort(testSet.begin(), testSet.end());

  BOOST_TEST_MESSAGE("sort (10 bit unsigned) ");
    sort(bc, be);
    
  BOOST_CHECK(equal(testSet.begin(), testSet.end(), bc));
}


BOOST_AUTO_TEST_CASE(sort_10_bit_signed)
{
  const size_t testSize = 512*1024; 
  BOOST_TEST_MESSAGE( "reg (signed) v 10 bits sort with " << testSize << " elements");
  vector<reg> testSet(testSize);  
  vector<reg>::iterator c(testSet.begin());
  vector<reg>::iterator e(testSet.end());
  const reg ten_bit_mask((1<<10) -1);
  vector<char> buf((testSize *10 + 7)/8);

  bit_int_itr<10,reg>  b0(bref(&buf.front()));
  bit_int_itr<10,reg>  bc(b0);

  for(; c < e; ++c, ++bc)
    {
      reg orig = rand() & ten_bit_mask;
      reg ten_bit_number = signextend<reg, 10>(orig);
      // we don't care what the orig number is, just what is interpreted
      // as when we take only ten bits of it.  This should approximate an
      // even split of positive and negative numbers
      *bc = *c = ten_bit_number;
    }
  bit_int_itr<10,reg> be(bc);
  bc = b0;
        
  BOOST_TEST_MESSAGE("sort reg");
  sort(testSet.begin(), testSet.end());

  BOOST_TEST_MESSAGE("sort (10 bit unsigned) ");
  sort(bc, be);
    
  BOOST_CHECK(equal(testSet.begin(), testSet.end(), bc));
}



BOOST_AUTO_TEST_CASE(example_1){

  alloced_bref p0(13 * 1);
  bit_int_itr<13,ureg> a(p0);
  bit_int_itr<13,ureg> e(a + 1); 
  *a = 40;
  // cout << *a << endl;             // 40
  BOOST_CHECK(*a == 40);
  // cout << (*a=42) << endl;        // 42
  BOOST_CHECK((*a=42) == 42);
  // cout << (*(--(++a)))  << endl;  // 42
  BOOST_CHECK(*(--(++a)) == 42);
  ++a;
  bit_int_itr<13,ureg> end_by_bref(p0 + 13*1);
  BOOST_CHECK(a == e && a == end_by_bref); 
}

BOOST_AUTO_TEST_CASE(example_2){
  alloced_bref buf(10*1024);
  bit_int_itr<10, ureg> beg(buf);
  bit_int_itr<10, ureg> end(beg + 1024);
  BOOST_CHECK((end - beg) == 1024);
  unsigned i = 0;
  for(auto cur = beg; cur != end; ++cur){ *cur = i++; }
  random_shuffle(beg, end);
  
  BOOST_TEST_MESSAGE("check to see that each value is represented exactly once");
  vector<ureg> buf2(1024/c_register_bits, 0);
  bref check(&buf2.front());  // dereferencing a bref acts as a bit vector
  for_each(beg, end, [&check](ureg val){
      BOOST_CHECK(*(check + val) == 0);
      (check + val).write(1, 1);
    });

  BOOST_TEST_MESSAGE("by virtue of the count and completeness, we have found and hit"
                     "every value [0,1024");
  
}

