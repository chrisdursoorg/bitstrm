//
// Created by chris on 8/29/24.
//

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "BitIntIterCompilation"

#include <boost/test/unit_test.hpp>
#include <bitstrm/bit_int_itr_details.hpp>
#include <bitstrm/alloced_bref.hpp>
#include <bitstrm/print.hpp>
#include <iostream>

using namespace std;
using namespace bitstrm;


BOOST_AUTO_TEST_CASE(example_iterator){

 #if 0
  cout << "BEGIN" << endl;
  reg R = c_register_bits;
  for(reg x = -1; x >= -2*R -1; --x){
    
    reg w = (x + 1) / R;
    reg r = (x + 1) % R;
    
    cout << x << " off: " << (R - 1 + r)   << "  ptr adj: " << w - 1   << endl;
  }
  cout << "END" << endl;
 #endif
  
  bref addr;

  iterator_example<int_wrapper<reg const>> ex0(addr, 4);
  auto  ex1 = ex0;
  
  BOOST_CHECK(ex0 == ex1);
  BOOST_CHECK(!(ex0 != ex1));
  ++ex0;
  BOOST_CHECK(ex0 != ex1++);
  BOOST_CHECK(ex0 == ex1);
  --ex0;
  BOOST_CHECK(ex0 != ex1--);
  BOOST_CHECK(ex0 == ex1);

  cout << "ex0: " << ex0 << " (before)" << endl;
  ex0 += 42;
  cout << "ex0: " << ex0 << " (after)" << endl;
  BOOST_CHECK(ex0 != ex1);
  ex0 -= 42;
  cout << "ex0: " << ex0 << " (restored)" << endl;
  cout << "ex1: " << ex1 << endl;
  BOOST_CHECK(ex0 == ex1);
 
}

BOOST_AUTO_TEST_CASE(bit_int_reference_test) {

    vector<bool> example;
    alloced_bref buf(20);
    nbit_int_reference<ureg> ref1(8, buf);
    nbit_int_reference<ureg> ref2(ref1);
    
    ref1 = 42;
    BOOST_CHECK(ref2                           == 42);
    BOOST_CHECK(ref1                           == ref2);
    // BOOST_CHECK(static_cast<int>(42)        == ref2); // 'std' implementation only
    BOOST_CHECK(!!static_cast<ureg>(ref2));
    // BOOST_CHECK(static_cast<bool>(ref2));             // 'std' implementation only
    BOOST_CHECK(!(ref2 < ref1));
    nbit_int_reference<reg> ref3(8, buf + 8);
    // BOOST_CHECK(ref3 == ref2);                        // cannot compare reg and ureg specialization
    nbit_int_reference<ureg> ref4(8, buf + 8);
    ref4 = 43;
    BOOST_CHECK(static_cast<reg>(static_cast<ureg>(ref4))
        == static_cast<reg>(ref3));                      // ok because 8-bit pattern for 43 is identical
    BOOST_CHECK(ref1 < ref4);
    swap(ref1, ref4);
    BOOST_CHECK(ref1 == 43);
    BOOST_CHECK(ref4 == 42);
    ureg five = 5;
    swap(ref4, five);
    BOOST_CHECK(ref4 == 5);
    BOOST_CHECK(five  == 42);
    swap(five, ref4);
    BOOST_CHECK(ref4 == 42);
    BOOST_CHECK(five == 5);
}
