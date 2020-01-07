// print_unitttest.cpp
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "BitStreamBref"

#include <boost/test/unit_test.hpp>
#include "bitstrm/alloced_bref.hpp"
#include "bitstrm/utility.hpp"
#include "bitstrm/bref.hpp"
#include "bitstrm/print.hpp"
#include <sstream>

using namespace bitstrm;
using namespace std;

BOOST_AUTO_TEST_CASE(null_bref)
{
  BOOST_TEST_MESSAGE("bref::print");

  bref n(nullptr);
  stringstream str;
  n.print(str);
  BOOST_CHECK(str.str() == "0x0 +0");

  alloced_bref a(1024);
  bref         aa=a;

  // S-O-S message
  for(int i = 0; i < 8; ++i){
    unsigned msg = (i%2 == 0) ? ((((1<<1)|1)<<1)|1): 0; // Mores 'S' and 'O' respectively 
    aa.iwrite(3, msg);
  }

  str.str("");
  print(str, a, aa);
  BOOST_CHECK(str.str() == "b111000111000111000111000");
}