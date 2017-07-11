#define BOOST_TEST_DYN_LINK

#define BOOST_TEST_MODULE "BitStreamReg"

#include <boost/test/unit_test.hpp>
#include "bitstrm/reg.hpp"
#include <limits>

using namespace boost::unit_test;
using namespace bitint;
using namespace std;

BOOST_AUTO_TEST_CASE( byte_endianness )
{
  BOOST_TEST_MESSAGE("A test for endianness operation, which may even by a noop");
  BOOST_REQUIRE(endian_adj(1));
  BOOST_REQUIRE(!endian_adj(0));
  BOOST_REQUIRE(numeric_limits<reg>::max() == endian_adj(endian_adj(numeric_limits<reg>::max())));
  BOOST_REQUIRE(numeric_limits<reg>::min() == endian_adj(endian_adj(numeric_limits<reg>::min())));
}

