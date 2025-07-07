#define BOOST_TEST_DYN_LINK

#define BOOST_TEST_MODULE "BitStreamReg"

#include <boost/test/unit_test.hpp>
#include "bitstrm/reg.hpp"
#include <limits>

using namespace boost::unit_test;
using namespace bitstrm;
using namespace std;

BOOST_AUTO_TEST_CASE( byte_endianness )
{
  BOOST_TEST_MESSAGE("Endianess, a feature that permits packing within byte alligned memory has been removed.  It adds complexity and was not thuroughly tested");
  
  BOOST_REQUIRE(true);
  // BOOST_REQUIRE(endian_adj(1));
}

