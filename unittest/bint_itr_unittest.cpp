// bint_itr_unittest.cpp

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "BintItrUnittest"

#include <boost/test/unit_test.hpp>
#include "bitstrm/bint_itr.hpp"
#include "bitstrm/alloced_bref.hpp"

using namespace boost::unit_test;
using namespace bitstrm;
using namespace std;


BOOST_AUTO_TEST_CASE(bint_itr_defintions){

  mutable_bint_itr<ureg> x;
  mutable_bint_itr<ureg> y(x);
  mutable_bint_itr<reg>  z;

  x = y;
  
  // z = y; // signed/unsigned mismatch

  const_bint_itr<ureg> xx;
  const_bint_itr<ureg> yy(xx);
  const_bint_itr<ureg> zz(x);
  
  xx = yy;
  xx = y;

  // xx = z;  // signed/unsigned mismatch
  // x = xx;  // can't make a const unconst

  BOOST_TEST_MESSAGE("x::value_type:  "     << typeid(decltype(x)::value_type).name());
  BOOST_TEST_MESSAGE("xx::value_type: "     << typeid(decltype(xx)::value_type).name());
  BOOST_TEST_MESSAGE("x::reference: "       << typeid(decltype(x)::reference).name());
  BOOST_TEST_MESSAGE("xx::reference: "      << typeid(decltype(xx)::reference).name());
  BOOST_TEST_MESSAGE("x::iterator_type: "   << typeid(decltype(x)::iterator_category).name());
  BOOST_TEST_MESSAGE("xx::iterator_type: "  << typeid(decltype(xx)::iterator_category).name());
  BOOST_TEST_MESSAGE("x::size_type: "       << typeid(decltype(x)::size_type).name());
  BOOST_TEST_MESSAGE("xx::size_type: "      << typeid(decltype(xx)::size_type).name());
  BOOST_TEST_MESSAGE("x::difference_type: " << typeid(decltype(x)::difference_type).name());
  BOOST_TEST_MESSAGE("xx::difference_type: "<< typeid(decltype(xx)::difference_type).name());

 }

BOOST_AUTO_TEST_CASE(bint_proxy_unittest){

  alloced_bref buf(42);
  buf.write(-123, 12);

  bint_proxy<reg> p {buf, 12};

  BOOST_CHECK(p.read() == -123);
  BOOST_CHECK(p == -123);
  p.write(123);
  BOOST_CHECK(p == 123);
  p = 111;
  BOOST_CHECK(p == 111);
  // bint_proxy<ureg> q(p); -- can't ctor from <reg> specialization
  bint_proxy<reg> q(p);
  BOOST_CHECK(q == 111);

  alloced_bref buf2(42);
  buf2.write(456, 12);
  bint_proxy<reg> r(buf2, 12);
  swap(p, r);
  BOOST_CHECK(p.read() == 456);
  BOOST_CHECK(r.read() == 111);
  reg lit = 1;
  swap(p, lit);
  BOOST_CHECK(lit == 456);
  BOOST_CHECK(p.read() == 1); 
  swap(lit, p);
  BOOST_CHECK(lit == 1);
  BOOST_CHECK(p.read() == 456);
  BOOST_CHECK(r < p);
  BOOST_CHECK(!(r == p));
  BOOST_CHECK(!(p == r));
  p = r;
  BOOST_CHECK(p == r);
}

BOOST_AUTO_TEST_CASE(bint_itr_simple_array_unittest){

  const int arbitrary_array_size = 1020;
  const int large_enough_bsize = 11;
  alloced_bref buf(arbitrary_array_size*large_enough_bsize);

  {
    mutable_bint_itr<ureg> a(buf, large_enough_bsize);
    const_bint_itr<ureg>   b = a;
    auto ae = a + arbitrary_array_size;

    for( unsigned i = 1; a != ae; ++i, ++a)
      *a = i;

    for( unsigned i = 1; b != ae; ++i, ++b)
      BOOST_CHECK(*b == i);
  }
  {
    mutable_bint_itr<reg> a(buf, large_enough_bsize);
    const_bint_itr<reg>   b = a;
    auto ae = a + arbitrary_array_size;

    for( int i = -4; a != ae; ++i, ++a)
      *a = i;

    for( int i = -4; b != ae; ++i, ++b)
      BOOST_CHECK(*b == i);
  }
 
}

BOOST_AUTO_TEST_CASE(navigation_unittest){
  {
    mutable_bint_itr<ureg> pos(nullptr, 4);
    mutable_bint_itr<ureg> oth(pos);

    BOOST_CHECK(pos == oth);
    BOOST_CHECK(pos++ == oth);
    BOOST_CHECK(pos != oth++);
    BOOST_CHECK(pos == oth);
    (pos += 6) += 6;
    BOOST_CHECK(oth < pos);
    BOOST_CHECK(pos > oth);
    BOOST_CHECK(pos - oth == 12);
    oth = oth + 12;
    BOOST_CHECK(pos == oth);
    oth = oth - 12;
    pos -= 12;
    BOOST_CHECK(pos == oth);
    BOOST_CHECK(pos >= oth);
    BOOST_CHECK(pos <= oth);
    BOOST_CHECK(++pos >= oth);
    BOOST_CHECK(--(--pos) <= oth);
    pos = oth;
    BOOST_CHECK(oth == pos);
  }

  {
    mutable_bint_itr<ureg> orig(nullptr, 4);
    const_bint_itr<ureg> pos(orig);
    const_bint_itr<ureg> oth(pos);

    BOOST_CHECK(pos == oth);
    BOOST_CHECK(pos++ == oth);
    BOOST_CHECK(pos != oth++);
    BOOST_CHECK(pos == oth);
    pos += 12;
    BOOST_CHECK(oth < pos);
    BOOST_CHECK(pos > oth);
    BOOST_CHECK(pos - oth == 12);
    oth = oth + 12;
    BOOST_CHECK(pos == oth);
    oth = oth - 12;
    pos -= 12;
    BOOST_CHECK(pos == oth);
    BOOST_CHECK(pos >= oth);
    BOOST_CHECK(pos <= oth);
    BOOST_CHECK(++pos >= oth);
    BOOST_CHECK(--(--pos) <= oth);
    pos = oth;
    BOOST_CHECK(oth == pos);
    oth = pos = orig;
    BOOST_CHECK(pos == orig);
  }
}

BOOST_AUTO_TEST_CASE(iterator_access){
  
  alloced_bref buf(8*12);
  bref fill(buf);
  
  for( int i = 1; i <= 12; ++i)
    fill.iwrite(i, 8);

  mutable_bint_itr<reg > a(buf, 8);
  const_bint_itr  <reg > b(a);
  mutable_bint_itr<ureg> c(buf, 8);
  const_bint_itr  <ureg> d(c);

  BOOST_CHECK( *a == *b and reg(*c) == *b and *c == *d);

  for(int i = 0; i < 12; ++i, ++b, ++d)
    BOOST_CHECK((a[i] == *b) and (*b == reg(*d)) and (c[i] == *d));

}

BOOST_AUTO_TEST_CASE(const_check){

  alloced_bref buf(42);
  mutable_bint_itr<ureg> readwriter(buf, 12);
  const_bint_itr<ureg>   reader(buf, 12);

  *readwriter = 11;
  // ureg& reftarget = *readwriter;  FAIL,  not a real reference to a ureg
  // ureg ltarget = *readwriter;
  // ltarget = 1;  compiler may warn you 'unused variable' as this does nothing
  BOOST_CHECK(*readwriter == 11);
  auto rtarget = *readwriter;
  rtarget = 12;                          //OK, rtarget type is bint_proxy 
  BOOST_CHECK(*readwriter == 12);
  BOOST_CHECK(*readwriter == *reader);   //OK, the 'buf' bits have changed
  ureg ltarget = *readwriter;
  BOOST_CHECK(ltarget == 12);
  // *reader = 88;  FAIL, lvalue required as left operand of assignment
  auto const& rrtarget = *reader;
  BOOST_CHECK(rrtarget == 12);
}
