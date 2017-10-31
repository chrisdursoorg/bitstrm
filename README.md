# Bitstrm

## Rational

Implementing codecs, variable codec serialization and algorithms requiring arbitary bit sized integers (or states) frequently results in having to rewrite bit access operators.  These operations can be difficult to correctly implement particularly with regards to boundry conditions, two's complement arithmetic and branch reduction.  The bitstrm library attempts to address each of these concerns while providing a clean interface.

## Implementation

Implemented as a header only library compatible to `-std=c++11`, this code offers lightweight compilation (e.g. even weighty `std::ostream` for print only comes in with optional `#include "/bitstrm/print.hpp"` ).  Optional example code and boost unit tests may be built with `cmake`.


## Features

* adaptable to different hardware, reg is defined as int64_t and it can be easily modified to be your native signed register
* pointer and iterator _lightweight accessor_ behavior (i.e. can work directly in std algorithms)
* integrated endian transform for integers to and from byte stream
* two's compliment implemented, with negative two's complitment "prefix" (as suffix notation necessitates a branch and gives you `+/- 0` )
* reduction of branch operations
* simple codec primatives

## Architecture

### Basic
`reg` and `ureg` defined as `int64_t` and `uint64_t` respectively serve as the upper integer size and as the internal working word.  Larger units are preferred for working with small number of bits as less fetching and word boundry stitching occurs with their codec.  Endian transformation assures that off word allocation works as expected, however user beware as memory tools will object (uninitialized memory read, unowned memory write) and non-atomic operations will occur on trailing bytes potentially in conflict with concurent operations. 

The `bref` class codecs to/from the bitstrm `reg`/`ureg` with the extent of reg/ureg bsize : [0,64] bits of externalized magnitude as though it were a pointer to an individual bit, but with methods for pulling that and subsequent bits as either a signed or unsigned integer value.

A close examination of externalized (as in functionally determined or externally stored) magnitude reveals that the magnitude _is_ part of the number itself hence numeric decomposition is a common exploit for compressive techniques.  Two supported examples of numeric decomposition are
* if the magnitude is bound (e.g. min_bits({<src_array>} => bsize) then only min_bits are required to store each element in sequence of src_array, and
* if the magnitude id known (e.g. a singular value, (min_bits(value + 1) - 1 ) => bsize) then run length specified encoding can be used 

In both of these techniques a magnitude of '0' _*always*_ translates to '0' and written as a non-operation.  Thus the common array of all zero's {0,0,0,...,0} and the individual zero value {0} with externalized magnitude code to {} 


Further Rational for `0` Width Integer:
> Intuitively for _2^i_ with _i_ being the index of the bits _i:{null, 0, 1, 2, ..}_ correspond to _bsize:{0, 1, 2, 3, ...}_ then _2^null <=> 0_ value, as in the _i=0_ case,  with _run length specified encoding_ the corresponding `0b0` does not imply the `0` value but rather the _(1 << i) - 1 + `0b0` <=> 1_ value, likewise `0b00`, `0b000`, ... need not similarly represent _0_.  They can represent other values! 
>
> ref:  `bref.hpp // run length

### Fixed Width Iterator 
Using a reference object (like `std::vector<bool>::iterator`) `bit_int_itr` integrates the bitstrm functionality directly into std and iterator-based algorithms with a little overhead.


## State

I am comfortable with `gcc` and `clang` compile on `x86` platform, and be very grateful for suggestions to root out any language unspecified behavior and/or unnecessary branching or further optimization.  

## Examples

### what can you do with up to 3 bits?
```
alloced_bref alloc0(3);
bref p0 = alloc0;             // make some 'pointers'
bref p1 = p0;
assert(p0 == p1);             // pointers allign
p0.iwrite(3, 6);              // write small unsigned value 6, incr p0
assert(p0 > p1);              // p0 past p1
assert(p0 - p1 == 3);         // by 3 bits
swap(p0, p1);                 // lets reorder pointers
assert(p0.read_ureg(3) == 6); // recover value
alloced_bref alloc1(60);      // 20x alloc is actually the same, underneath a uint64
assert(alloc0 != alloc1);     // alloc1 wrt alloc0 cannot coincide
bref p2 = alloc1;
bref p3 = copy(p0, p1, p2);    // copy the underlying bits
assert(p3 - p2 == 3);          // (all three of them)
assert(p0.read_ureg(3)
        == p2.read_ureg(3));   // the value got copied
// note that with three bits one can store {-4, -3, -2, -1, 0, 1, 2, 3}
p0.write(3, -2);               // store -2 into alloc0
assert(p0.read_reg(3) == -2);  // writing keeps sign, reading must specifiy fmt
assert(p0.read_reg(3)
        == p2.read_reg(3));    // what gives?! while -2 != 6,
                               // -2 == reg(6U) when your register is 3-bit

// back to the world of whole numbers,
// of the subset of those of size is 3 bits width,
// whats the maximum value you can store?
p0.write(3, 7 /*or 0b111*/);
assert(bref(p0).iread_rls(3) == 14); // kind of strange?!
p2.write(3, 0 /*or 0b000*/);         // here's a hint
assert(bref(p2).iread_rls(3) == 7);  // the minimum value is not zero but 7
assert(bref(p2).iread_rls(0) == 0);  // and, zero takes *0* bits to store,  or {0}
assert(bref(p0).iread_rls(2) == 6 && bref(p2).iread_rls(2) == 3);  // the range for 2 bit rls
assert(bref(p0).iread_rls(1) == 2 && bref(p2).iread_rls(1) == 1);  // the range for 1 bit rls
// or {0}{1, 2}{3, 4, 5, 6}{7, 8, 9, 10, 11, 12, 13, 14},
// for ranges of 0, 1, 2, and 3 bits respectively
```
### simple bit_int_itr example

```
  alloced_bref p0(13 * 1);          // allocate for 1 13 bit value
  bit_int_itr<13,ureg> a(p0);
  bit_int_itr<13,ureg> e(a + 1); 

  *a = 40;                          // [0] write that value
                                    // [1] assign and dereference value
                                    // [2] walk and dereference value again
  cout << *a << endl;               // > 40
  cout << (*a=42) << endl;          // > 42
  cout << (*(--(++a)))  << endl;    // > 42
  ++a;
  bit_int_itr<13,ureg> end_by_bref(p0 + 13*1);
  assert(a == e && a == end_by_bref); 
```
### bit_int_iter Create and Check Random 10 Bit Complete Table 
This time create two tables @ buf and @ buf2, where buf contains complete set of 10 bit values in randomized sequence. The buf2 table is a bit vector used to verify that the 10 bit table is whole and complete.

```
  alloced_bref buf(10*1024);               // allocate space for 1024 10 bit values
  bit_int_itr<10, ureg> beg(buf);   
  bit_int_itr<10, ureg> end(beg + 1024);
  assert((end - beg) == 1024);            
  unsigned i = 0;                          // assign {0, 1, 2, ..., 1023}
  for(auto cur = beg; cur != end; ++cur){ *cur = i++; }
  random_shuffle(beg, end);
                                           // use a bit vector
                                           // for variety and to fill to 0
  vector<ureg> buf2(1025/c_register_bits, 0);
  bref check(&buf2.front());               
  for_each(beg, end, [&check](ureg val){
      assert(*(check + val) == 0);
      (check + val).write(1, 1);
    });

  assert(true && "passing each of the above assertions (1024) of them, "
                 "have set each and every bit exactly once, hence completeness "
                 "proven to original assignment");
  
  BOOST_TEST_MESSAGE("by virtue of the count and completeness, we have found and hit"
                     "every value [0,1024)");

```


