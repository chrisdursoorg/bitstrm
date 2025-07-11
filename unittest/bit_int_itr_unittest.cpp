// bit_int_itr_details_unittest.cpp
//

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "BitIntIterDetails"

#include <boost/test/unit_test.hpp>
#include "bitstrm/alloced_bref.hpp"
#include "bitstrm/reg.hpp"
#include "bitstrm/bit_int_itr.hpp"
#include <random>

using namespace boost::unit_test;
using namespace bitstrm;
using namespace std;

BOOST_AUTO_TEST_CASE(ctors) {

  bref arbitrary;

  // bit_int_itr<ureg>  an_unsigned_bit_int_itr;   if bwidth is going to be static, then you must specify it
  bit_int_itr<12, ureg>  an_unsigned_bit_int_itr;
  bit_int_itr<12, reg>   a_signed_bit_int_itr;
  bit_int_citr<12, reg>  a_signed_bit_int_citr;
  // an_unsigned_bit_int_itr = a_signed_bit_int_itr;  violated signed/unsigned ness
  a_signed_bit_int_citr = a_signed_bit_int_itr;

  bit_int_citr<12, ureg>  an_unsigned_bit_int_citr;


  dbit_int_itr<ureg>  ok_to_have_dynamic_bsize_but_uninitialized;
  dbit_int_citr<ureg> uninitialized_dynamic_and_const;
  uninitialized_dynamic_and_const = ok_to_have_dynamic_bsize_but_uninitialized;


  dbit_int_citr<ureg> make_it_42_wide(arbitrary, 42);
  // ok_to_have_dynamic_bsize_but_uninitialized = make_it_42_wide;  violated assign const to nonconst
  // dbit_int_itr<ureg> disallowed(make_it_42_wide);                violated constuct a nonconst from a const
  uninitialized_dynamic_and_const = make_it_42_wide;
  BOOST_TEST(make_it_42_wide.bwidth() == 42);
  BOOST_TEST(uninitialized_dynamic_and_const.bwidth() == 42);
  BOOST_TEST(an_unsigned_bit_int_itr.bwidth() == 12);
  // make_it_42_wide = an_unsigned_bit_int_itr;  // throws an assert as bwidth  do not match
}

BOOST_AUTO_TEST_CASE(example1) {

  alloced_bref p0(13 * 1);
  bit_int_itr<13,ureg> a(p0);

  *a = 40;
  BOOST_TEST(*a    == 40);
  BOOST_TEST(*a=42 == 42);
  BOOST_TEST((*(--(++a))) == 42);
}



BOOST_AUTO_TEST_CASE(example2) {

  alloced_bref buf(10*1024);
  bit_int_itr<10, ureg> beg(buf);
  bit_int_itr<10, ureg> end(beg + 10*1024);
  BOOST_TEST(distance(beg,end) == 1024);

#if 0
  vector<unsigned> ans(distance(beg, end));

  unsigned i = 0;
  for_each(beg, end, [&i](unsigned v){ v = i++;});
  random_shuffle(beg, end);
#endif
}
#if 0
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
  buf .iwrite(sentry, 8);                   // mark the ends for later;
  buf2.iwrite(sentry, 8);
  (buf  +  numbers_bsize).write(sentry, 8);
  (buf2 +  numbers_bsize).write(sentry, 8);
  

  bit_int_itr<13,reg> bc(buf2);
  bit_int_itr<13,reg> be(buf2+numbers_bsize);
  BOOST_TEST_MESSAGE("mechanism 1 to code container values into bitstrm backed by buf");
  for_each(container.begin(), container.end(), [bitSz,&buf](int v) { buf.iwrite(v, bitSz); });
  BOOST_TEST_MESSAGE("mechanism 2 to code container values into bitstrm backed by buf2");
  BOOST_CHECK(be == copy(container.begin(), container.end(), bc));
  buf .reset();
  buf2.reset();
  BOOST_CHECK(buf.iread<reg>(8) == sentry && buf2.iread<reg>(8) == sentry);
  BOOST_CHECK((buf  + numbers_bsize).iread<reg>(8) == sentry);
  BOOST_CHECK((buf2 + numbers_bsize).iread<reg>(8) == sentry);
  BOOST_TEST_MESSAGE("lots of different ways to be equal");
  buf .reset();
  buf2.reset();
  BOOST_CHECK_MESSAGE(equal(buf, buf + 2*8 + numbers_bsize, buf2), "the bitstrms should be equal");
  BOOST_CHECK_MESSAGE(equal(bc, be, container.begin()), "the itr should return the original sequence");
}

BOOST_AUTO_TEST_CASE(dynamic_interface_style_itr)
{
  // bit_int_itr performs read on assignment, memory must be allocated
  const int arbitrary_bsize = 4;
  const int maximum_elements = 5;
  alloced_bref any(arbitrary_bsize*maximum_elements); 
  dbit_int_itr<ureg> begin(any, arbitrary_bsize);
  dbit_int_itr<ureg> empty_end(begin);
  dbit_int_itr<ureg> two_end(any + arbitrary_bsize*2, arbitrary_bsize);
  dbit_int_itr<ureg> two_by_itr_end(begin + 2);
  
  // address checking
  BOOST_CHECK( two_end == two_by_itr_end);
  BOOST_CHECK( begin == empty_end);
  BOOST_CHECK( begin < two_end );
  BOOST_CHECK( begin + 1 < two_end );
  BOOST_CHECK( begin + 2 == two_end );
  BOOST_CHECK( begin != two_end );
  BOOST_CHECK( two_end - begin == 2);
  dbit_int_itr<ureg> t = begin  + 1;
  BOOST_CHECK( --t == begin);
  BOOST_CHECK( t + 2 == two_end );
  BOOST_CHECK( t     == begin);
  BOOST_CHECK( t++   == begin);
  BOOST_CHECK( --t   == begin);
  BOOST_CHECK( t++   == begin);
  BOOST_CHECK( ++t   == two_end);

  // reuse old buffer with move
  alloced_bref other2(std::move(any));
  alloced_bref other = std::move(other2);
  begin   = dbit_int_itr<ureg>(other, arbitrary_bsize);
  dbit_int_itr<ureg> cur(begin);
  dbit_int_itr<ureg> end(other + arbitrary_bsize*5, arbitrary_bsize);
  for(int i = 0; cur != end; ++cur, ++i){
    *cur = i;
    BOOST_CHECK(cur - begin == i);
  }
  
  // read it
  cur = begin;
  for(unsigned i = 0; cur != end; ++cur, ++i)
    BOOST_CHECK(*cur == i);
  
}

template<class ITR, int ...SECOND_ARG>
void check_itr(){

  // initializing arrays
  typedef  std::vector<int> containerType;
  containerType container{1,2,3,4,4,5,6,7,7,7,7};

  const unsigned intSz = 4;
  const unsigned sentrySz = 17;
  const int sentry = 4242;   // set "magic number to act as front/back sentry
  const unsigned endOfDataOff = (container.size()*intSz + sentrySz);
  BOOST_CHECK(min_bits(container.begin(), container.end()) <= intSz);
  const unsigned bsize = container.size()*intSz + 2*sentrySz; 
  alloced_bref buf(bsize);
  alloced_bref buf2(bsize);

  // write front and back sentry
  buf .write(sentry, sentrySz);
  buf2.write(sentry, sentrySz);     
  (buf + endOfDataOff).iwrite(sentry, sentrySz);
  (buf2+ endOfDataOff).iwrite(sentry, sentrySz);
  
  bref p(buf+sentrySz);                  // bitstreams start following sentry
  bref p2(buf2+sentrySz);
  bref p2e(p2 + container.size()*intSz); //  end at sentry

  ITR bc(p2, SECOND_ARG...);
  ITR be(p2e, SECOND_ARG...);

  BOOST_TEST_MESSAGE("mechanism 1 to code container values into bitstrm backed by buf");
  for_each(container.begin(), container.end(), [&p](containerType::value_type value) { p.iwrite(value, intSz); });
  BOOST_TEST_MESSAGE("mechanism 2 to code container values into bitstrm backed by buf2");
  BOOST_CHECK(be == copy(container.begin(), container.end(), bc));
  BOOST_CHECK(buf.read<reg>(sentrySz) == sentry && buf2.read<reg>(sentrySz) == sentry);
  
  BOOST_CHECK((buf + endOfDataOff).iread<reg>(sentrySz) == sentry);
  BOOST_CHECK(p2e.read<reg>(sentrySz) == sentry);
  BOOST_TEST_MESSAGE("lots of different ways to be equal");
  BOOST_CHECK_MESSAGE(equal(p2, p2e, ((bref&)(buf)+sentrySz)), "the bitstrms should be equal");
  BOOST_CHECK_MESSAGE(equal(bc, be, container.begin()), "buf should return the original data");
}

BOOST_AUTO_TEST_CASE( store_values_two_different_ways_boost_style )
{
  const unsigned intSz = 4; 
  check_itr<bit_int_itr<intSz, ureg> >();
}


BOOST_AUTO_TEST_CASE( store_values_two_different_with_dynamic_bsize_ways_boost_style )
{
  const int bitSz = 4;
  check_itr<dbit_int_itr<ureg>, bitSz >();
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
  unsigned sentrySz = 8;
  buf.write(sentry, sentrySz);
  (buf + sentrySz + numbers_bsize).write(sentry, sentrySz);
  
  bref p(buf + sentrySz); 
  bref p0(p);                            // bitstream to start after "magic number"
  bref pe(p0 + numbers_bsize);           // and continues until back "magic number"

  bit_int_itr<13,reg> bc(p0);
  bit_int_itr<13,reg> be(pe);
  
  for_each(container.begin(), container.end(), [bitSz,&p](int value) { p.iwrite(value, bitSz); });
  stringstream str;
  str << "pre-sort: "; for_each(bc, be, [&str](reg v ){ str << v << ", ";});   str << endl;
  BOOST_TEST_MESSAGE(str.str());
  BOOST_CHECK(equal(container.begin(),container.end(), bc));
  sort(bc, be);
  BOOST_CHECK(buf.read<reg>(sentrySz) == sentry);
  BOOST_CHECK(pe .read<reg>(sentrySz) == sentry);
  str.str("");
  str << "pst-sort: "; for_each(bc, be, [&str](reg v ){ str << v << ", ";});   str << endl;
  BOOST_TEST_MESSAGE(str.str());
  sort(container.begin(), container.end());
  BOOST_CHECK(equal(container.begin(),container.end(), bc));
}

BOOST_AUTO_TEST_CASE(bit_int_itr_ops)
{
  // initializing arrays
  typedef  std::vector<int> int_vec;
  int_vec container{1,2,3,4,-4,5,6,7,192,323,3223};
  unsigned bitSz = min_bits(container.begin(),container.end());
  constexpr unsigned sentry = 4242;
  unsigned sentrySz = 17;
  BOOST_CHECK(bitSz == 13);
  unsigned const paySz = bitSz*container.size();
  alloced_bref const buf(paySz+2*sentrySz);

  bref p(buf + sentrySz);
  bref p0(p);
  bref pe(p + paySz);

  buf.write(sentry, sentrySz);
  pe .write(sentry, sentrySz);

  for_each(container.begin(), container.end(), [bitSz,&p](int_vec::value_type value) { p.iwrite(value, bitSz); });
  BOOST_CHECK(p == pe);
  BOOST_CHECK(buf.read<reg>(sentrySz) == sentry);
  BOOST_CHECK(pe.read <reg>(sentrySz) == sentry);
  BOOST_CHECK(paySz == static_cast<unsigned>(p - p0));

  // use std and bit_int_itr interators to check values
  int_vec::const_iterator b = container.begin();
  int_vec::const_iterator e = container.end();
  p = p0;
  BOOST_CHECK( bitSz == 13); //  "A mix of lvalue and dynamic, fine as long as they remain the same"); 
  bit_int_itr<13,reg> verify(p);
  for(; b != e; ++b, ++verify)
    BOOST_CHECK(*verify == *b);

  typedef bit_int_itr<13,reg> bit_13_s;
  bit_13_s bc(p0);
  bit_13_s be(pe);

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
  bit_13_s::swap(*(be-1), *(be-2));
  BOOST_CHECK( buf.read<reg>(sentrySz) == sentry);
  BOOST_CHECK( pe .read<reg>(sentrySz) == sentry);
  BOOST_CHECK(*(be-1) == 323);
  BOOST_CHECK(*(be-2) == 3223);
  swap(*(be-1), *(be-2)); // restore to original order/values
  BOOST_CHECK( buf.read<reg>(sentrySz) == sentry);
  BOOST_CHECK( pe .read<reg>(sentrySz) == sentry);
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
  BOOST_CHECK( buf.read<reg>(sentrySz) == sentry);
  BOOST_CHECK( pe .read<reg>(sentrySz) == sentry);
  sort(bc, be);
  BOOST_CHECK( buf.read<reg>(sentrySz) == sentry);
  BOOST_CHECK( pe .read<reg>(sentrySz) == sentry);
  str.str("");
  str << "pst-sort: ";  for_each(bc, be, [&str](reg v ){ str << v << ", ";});   str << endl;
  BOOST_TEST_MESSAGE(str.str());
  sort(container.begin(), container.end());
  BOOST_CHECK(equal(container.begin(),container.end(), bc));
}

BOOST_AUTO_TEST_CASE(itr_small_case){
  vector<unsigned> test{{21 ,77 ,485 ,497 ,502 ,667 ,688 ,708 ,744 ,825 ,906 ,1015}};
  alloced_bref buf((test.size())*10);
  bit_int_itr<10,ureg>  b0(buf);
  bit_int_itr<10,ureg>  be = copy(test.begin(), test.end(), b0);
  sort(test.begin(), test.end());
  sort(b0, be);
  BOOST_CHECK(be-b0 == long(test.size()));
  BOOST_CHECK(equal(test.begin(), test.end(), b0));
}

BOOST_AUTO_TEST_CASE(little_10_bit_sort){
  vector<ureg> testSet;
  
  for(int i = 0; i < 13; ++i){
    const size_t testSize = i; 
    BOOST_TEST_MESSAGE( "ureg v 10 bits sort with " << testSize << " elements");
    testSet.resize(testSize);  
    vector<ureg>::iterator c(testSet.begin());
    vector<ureg>::iterator e(testSet.end());
    const ureg ten_bit_mask((1<<10) -1);
  
    alloced_bref buf((testSize)*10);
    bit_int_itr<10,ureg>  b0(buf);
    bit_int_itr<10,ureg>  bc(b0);

    for(; c < e; ++c, ++bc)
      *bc = *c = (ureg)rand() & ten_bit_mask;
  
    bit_int_itr<10,ureg> be(bc);
    bc = b0;
  
    BOOST_CHECK(size_t(be - bc) == testSize);
  
    BOOST_TEST_MESSAGE("sort ureg");
    sort(testSet.begin(), testSet.end());

    BOOST_TEST_MESSAGE("sort (10 bit unsigned) ");
    sort(bc, be);
    BOOST_CHECK(equal(testSet.begin(), testSet.end(), bc));
  }
}

BOOST_AUTO_TEST_CASE(sort_10_bit_unsigned)
{
  const size_t testSize = 512*1024; 
  BOOST_TEST_MESSAGE( "ureg v 10 bits sort with " << testSize << " elements");
  vector<ureg> testSet(testSize);  
  vector<ureg>::iterator c(testSet.begin());
  vector<ureg>::iterator e(testSet.end());
  const ureg ten_bit_mask((1<<10) -1);
  
  alloced_bref buf((testSize)*10);
  bit_int_itr<10,ureg>  b0(buf);
  bit_int_itr<10,ureg>  bc(b0);

  for(; c < e; ++c, ++bc)
    *bc = *c = (ureg)rand() & ten_bit_mask;
    
  bit_int_itr<10,ureg> be(bc);
  bc = b0;

  BOOST_CHECK((be - bc) == testSize);
        
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

  random_device rd;
  mt19937 g(rd());
  shuffle(beg, end, g);
  
  BOOST_TEST_MESSAGE("check to see that each value is represented exactly once"
                     );
  vector<ureg> buf2(1024/c_register_bits, 0);
  bref check(&buf2.front());  // dereferencing a bref acts as a bit vector
  for_each(beg, end, [&check](ureg val){
      BOOST_CHECK(*(check + val) == 0);
      (check + val).write(1, 1);
    });

  BOOST_TEST_MESSAGE("by virtue of the count and completeness, we have found "
                     "and hit every value [0,1024");
  
}

BOOST_AUTO_TEST_CASE(code_example_1){

  BOOST_TEST_MESSAGE("Complete ten bit shuffled table constructed an verified");
  alloced_bref buf(10*1024);
  bit_int_itr<10, ureg> beg(buf);
  bit_int_itr<10, ureg> end(beg + 1024);
  
  unsigned i = 0;
  for(auto cur = beg; cur != end; ++cur, ++i)
    *cur = i;

  random_device rd;
  mt19937 g(rd());
  shuffle(beg, end, g);

  // fill check array with zero's 
  alloced_bref checkbuf(1024);
  bit_int_itr<1, ureg> check(checkbuf);
  for(auto e = check + 1024; check != e; ++check){ *check = 0;}
  
  bit_int_citr<10, ureg> cbeg(beg);
  bit_int_citr<10, ureg> cend(end);

  check = bit_int_itr<1, ureg>(checkbuf);
  for(;cbeg != cend; ++cbeg){
    // assert(check[*cbeg]++ == 0); would seem elegent
    // but we have to do this in three statements
    auto ref_proxy = *(check + *cbeg);
    BOOST_CHECK_MESSAGE( ref_proxy == 0, "value: " << ref_proxy << " place: " << *cbeg);
    ref_proxy = 1;
  }

    BOOST_TEST_MESSAGE("by virtue of the count and completeness, we have found "
                     "and hit every value [0,1024");
}

BOOST_AUTO_TEST_CASE(code_example_2){

  int k = 10;
  alloced_bref buf(k*1024);
  dbit_int_itr<ureg> beg(buf, k);
  dbit_int_itr<ureg> end(beg + (1 << k));
  unsigned i = 0;
  for(auto cur = beg; cur != end; ++cur){ *cur = i++; }

  random_device rd;
  mt19937 g(rd());
  shuffle(beg, end, g);

  // fill check_ array with zero's
  alloced_bref checkbuf(1024);
  bit_int_itr<1, ureg> check_(checkbuf);
  for(auto e = check_ + 1024; check_ != e; ++check_){ *check_ = 0;}

  dbit_int_citr<ureg> cbeg(beg);
  dbit_int_citr<ureg> cend(end);

    check_ = bit_int_itr<1, ureg>(checkbuf);
  for(;cbeg != cend; ++cbeg){
    // assert(check_[*cbeg]++ == 0); would seem elegent
    // but we have to do this in three statements
    reg ref_proxy = *(check_ + *cbeg);
    BOOST_CHECK_MESSAGE( ref_proxy == 0, "value: " << ref_proxy << " place: " << *cbeg);
    ref_proxy = 1;
  }

  BOOST_TEST_MESSAGE("by virtue of the count and completeness, we have found "
                     "and hit every value [0,1024");
  
}

BOOST_AUTO_TEST_CASE(const_itr_2_itr_test){

  bref empty(nullptr);
  bit_int_itr<3, int> i3type0(empty);
  bit_int_itr<3, int> i3type1(empty);

  bit_int_citr<3, int> i3ctype0(empty);
  bit_int_citr<3, int> i3ctype1(empty);

  //  i3type0 = i3ctype0;  a compiler error, cannot assign a const itr to a non const itr
  i3type0 = i3type1;    // same type
  BOOST_CHECK(i3type0 == i3type1);
  i3ctype1 = i3ctype0;  // same type
  BOOST_CHECK(i3ctype1 == i3ctype0);

  i3ctype1 = i3type0;   // ok to copy a non const itr into a const itr

  BOOST_CHECK(i3ctype1 == i3type0);

}
#endif