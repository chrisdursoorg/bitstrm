// consolidate_perf.cpp
//

#include <iostream>
#include <cstdlib>
#include <cstdint>
#include <limits>
#include <type_traits>
#include "bitstrm/TimeFixture.hpp"
#include "bitstrm/bitstrm.hpp"
#include "bitstrm/bint_itr.hpp"
#include "bitstrm/alloced_bref.hpp"

using namespace std;
using namespace bitstrm;

unsigned TimeFixture::s_timer_number;

template<class I>
I cycle16(I v) {return is_signed_v<I> ? v % 16 - 8 : v % 16; }

int main(int /*argc*/ , const char** /*argv*/){

  cout << 
    "\nGiven an input stream of integers with a bounding maximal extent,\n"
    "consolidate the values into a discovered smaller stream.  Natuarlly\n"
    "bitstrm may take less space but how much a performance hit or \n"
    "gain will result?\n" << endl;

  auto print = [](auto n)  { std::cout << n << ", "; };
  
  cout << "example data:\n"; 
  {
    vector<ureg> a(32);
    ureg i = 0;
    generate(a.begin(), a.end(), [&i](){ return cycle16(i++);});
    for_each(a.begin(), a.end(), print); cout << "..."  << endl;
  }
  cout << "and" << endl;
  {
    vector<reg> a(32);
    reg i = 0;
    generate(a.begin(), a.end(), [&i](){ return cycle16(i++);});
    for_each(a.begin(), a.end(), print); cout << "..." <<  endl;
  }
  
  
  {
    long test_size = 1024*1024*64; 
    cout << "\nTest 64 -> 8 bits v 64 -> 4 bits test_size: " << test_size
	 << endl << endl;
    {
      vector<uint64_t> a(test_size);
      vector<uint8_t>  b(a.size());
      alloced_bref     c(a.size()*4);
      ureg i{0};
      generate(a.begin(), a.end(), [&i](){ return cycle16(i++);});
      { TimeFixture copy_("unsigned copy to 8 bits");
	copy(a.begin(), a.end(), b.begin());
      }
      { TimeFixture copy_("unsigned copy to 4 bits");
	copy(a.begin(), a.end(), mutable_bint_itr<ureg>(c, 4));
      }
      { TimeFixture check("unsigned check of 8 bits");
	ureg i{0}; for_each(b.begin(), b.end(), [&i](auto v)
	{ if(cycle16(i++) != v) cout << "error" << endl; });
      }
      { TimeFixture check("unsigned check of 4 bits");
	const_bint_itr<ureg>d(c, 4);
	const_bint_itr<ureg>e(d + a.size());
	ureg i{0}; for_each(d, e, [&i](auto v)
	{ if(cycle16(i++) != v) cout << "error" << endl; });
      }
    }

    vector<int64_t> a(test_size);
    vector<int8_t>  b(a.size());
    alloced_bref    c(a.size()*4);
    
    { 
      reg i{0};
      generate(a.begin(), a.end(), [&i](){ return cycle16(i++);});
      { TimeFixture copy_("signed copy to 8 bits");
	copy(a.begin(), a.end(), b.begin());
      }
       { TimeFixture copy_("signed copy to 4 bits");
	copy(a.begin(), a.end(), mutable_bint_itr<ureg>(c, 4));
       }
      { TimeFixture check("signed check of 8 bits");
	reg i{0}; for_each(b.begin(), b.end(), [&i](auto v)
	{ if(cycle16(i++) != v) cout << "error" << endl;});
      }
      { TimeFixture check("signed check of 4 bits");
	const_bint_itr<reg>d(c, 4);
	const_bint_itr<reg>e(d + a.size());
	reg i{0}; for_each(d, e, [&i](auto v)
	{ if(cycle16(i++) != v) cout << "error" << endl; });
      }
    }
     
  }
  cout << endl;  
  return 0;
}



