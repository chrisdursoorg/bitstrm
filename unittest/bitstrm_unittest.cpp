// bitstrm_test.cpp

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "BitStreamBitStream"

#include <boost/test/unit_test.hpp>
#include "bitstrm/alloced_bref.hpp"
#include "bitstrm/utility.hpp"
#include <limits>

using namespace boost::unit_test;
using namespace bitstrm;
using namespace std;

BOOST_AUTO_TEST_CASE(arbitary_table_and_reset){
  alloced_bref strm(1024*17);
  ureg i = 0;
  const unsigned mask17bit = mask(17);
  BOOST_TEST_MESSAGE("fill up a 17 bit table");
  while(!(mask17bit & i))
    strm.iwrite_(i++, 17);

  // as a bitstrm (instead of simple bref) it can be reset
  strm.reset();

  BOOST_TEST_MESSAGE("verify 17 bit table");
  i = 0;
  while(!(mask17bit & i))
    BOOST_CHECK(strm.iread<ureg>(17) == i++);
  
}

BOOST_AUTO_TEST_CASE(late_allocate_and_swap){
  alloced_bref strm;
  strm.allocate(1024*17);
  ureg i = 0;
  const unsigned mask17bit = mask(17);
  BOOST_TEST_MESSAGE("fill up a 17 bit table");
  while(!(mask17bit & i))
    strm.iwrite_(i++, 17);

  // a quick swap operation moves the underlying bits to strm2
  alloced_bref strm2;
  strm2.swap(strm);
  // keep track of curent reference position as it would have
  // moved too
  bref end = strm2;
  strm2.reset();
  // verify that extent given by end was correct
  BOOST_CHECK(end - strm2 == reg(17*i));

  BOOST_TEST_MESSAGE("verify 17 bit table");
  i = 0;
  while(!(mask17bit & i))
    BOOST_CHECK(strm2.iread<ureg>(17) == i++);
}

