// bitstrm_test.cpp

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "BitStreamBitStream"

#include <boost/test/unit_test.hpp>
#include "bitstrm/bitstrm.hpp"
#include "bitstrm/alloced_bref.hpp"
#include "bitstrm/utility.hpp"
#include "bitstrm/print.hpp"
#include <limits>
#include <cstdlib>

using namespace boost::unit_test;
using namespace bitstrm;
using namespace std;


bref create_test_pattern_pop_count(alloced_bref& dest, ureg& popcount){

  bref c = dest;
  ureg bsz = 0;
  for(unsigned i = 0; ((c-dest) + (bsz=bref::bsize(i)))  < dest.bsize(); ++i){
    c.iwrite(i, bsz);
    popcount += op_pop_count(i);
  }

  return c;
}

bref create_test_pattern(alloced_bref& dest){
  ureg ignore_count = 0;
  return create_test_pattern_pop_count(dest, ignore_count);
}



BOOST_AUTO_TEST_CASE(copy_dest_aligned){

  stringstream sstr, dstr;

  for(unsigned sz = 1234; sz < 12345; sz += 123){

    alloced_bref src(sz);
    alloced_bref dst(sz);

    bref e = create_test_pattern(src);
    bitstrm::copy(src, e, dst);
    BOOST_CHECK(sstr.str() == dstr.str());
  }
}

BOOST_AUTO_TEST_CASE(copy_dest_unaligned){

  stringstream sstr, dstr;

  for(unsigned sz = 634; sz < 850; sz += 123){

    alloced_bref src(sz);
    alloced_bref dst(sz + sz % 64);
    bref d = dst + sz % 64;
    
    bref e = create_test_pattern(src);

    print(sstr, src, e);
    bitstrm::copy(src, e, d);
    print(dstr, d, d + (e - src));

    BOOST_CHECK(sstr.str() == dstr.str());
  }
}

BOOST_AUTO_TEST_CASE(very_small_copy_aligned){
  
  alloced_bref very_small_src (c_register_bits);
  alloced_bref very_small_dest(c_register_bits);

  bref e = create_test_pattern(very_small_src);
  
  bitstrm::copy(very_small_src, e, very_small_dest);
  BOOST_CHECK(bitstrm::equal(very_small_src, e, very_small_dest));
}


BOOST_AUTO_TEST_CASE(g_very_small_copy_unaligned){

  alloced_bref very_small_src (c_register_bits);
  alloced_bref very_small_dest(c_register_bits + 42);
  very_small_dest += 42;

  bref e = create_test_pattern(very_small_src);
  
  bitstrm::copy(very_small_src, e, very_small_dest);
  BOOST_CHECK(bitstrm::equal(very_small_src, e, very_small_dest));
}

BOOST_AUTO_TEST_CASE(check_matched_mismatches){

  for(unsigned i = 0; i < 1024*1024; i += 4001){
    alloced_bref a(i);
    alloced_bref b(i);
    bref ae = create_test_pattern(a);
    bref be = create_test_pattern(b);
    BOOST_CHECK(mismatch(a, ae, b) == std::make_pair(ae, be));
  }
}

BOOST_AUTO_TEST_CASE(check_mismatched_mismatches){

  for(unsigned i = 1; i < 1024*1024; i += 4001){
    alloced_bref a(i);
    alloced_bref b(i);
    bref ae = create_test_pattern(a);
    create_test_pattern(b);
    unsigned random_salt_at = rand() % i;
    bref const salt_place = a + random_salt_at;
    salt_place.write(~salt_place.read<ureg>(1), 1);
    BOOST_CHECK(mismatch(a, ae, b) == std::make_pair
		(salt_place, b + random_salt_at)
		);
  }
}

BOOST_AUTO_TEST_CASE(check_clz){


  alloced_bref z(0);
  BOOST_CHECK(clz(z.bbegin(), z.bend()) == z.bend());

  alloced_bref t(12345);
  t.zero();
  BOOST_CHECK(clz(t.bbegin(), t.bend()) == t.bend());
  
  (t.bend()-1).iwrite(1,1);
  BOOST_CHECK(clz(t.bbegin(), t.bend()) == (t.bend() -1));

  t.write(1,2);
  BOOST_CHECK(clz(t.bbegin(), t.bend()) == t.bbegin() + 1);

  t.write(1,1);
  BOOST_CHECK(clz(t.bbegin(), t.bend()) == t.bbegin());

  
  alloced_bref a(1024*1024);
  a.zero();
  for(int i = a.bsize() - 1 ; i > 0; i -= 1234){
    (a + i).write(1, 1);
    BOOST_CHECK(clz(a.bbegin(), a.bend()) == a.bbegin() + i);
  }
}

// popcount
//
BOOST_AUTO_TEST_CASE(check_popcount){

  alloced_bref buf(1024*1024);

  do{
    ureg written_popcount = 0;
    bref e = create_test_pattern_pop_count(buf, written_popcount);
    BOOST_CHECK(popcount(buf.bbegin(), e) == written_popcount);
    buf.resize(buf.bsize()/33);
  } while(buf.bsize() != 0);

  // empty case
  BOOST_CHECK(popcount(buf.bbegin(), buf.bend()) == 0 );  
}
