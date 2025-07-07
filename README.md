# Bitstrm

Implementing codecs, variable codec serialization and algorithms requiring arbitrary bit sized integers (or states) frequently result in having to rewrite bit access operations.  These operations can be difficult to correctly implement particularly with regards to boundary conditions, two's complement arithmetic and branch reduction.  The bitstrm library attempts to address these concerns while providing a clean interface.

## Version Notes

v1.0.0  - Classic bitstrm, a formally versioned release prior to removal of now deprecated functionality and changed interface.

v2.0.0  - Changes improve the interface by discarding trivial functionality and by making more consistent arguments in others.

**Upgrading to v2.  The central method `bref::iwrite(size, value)` has had arguements swapped to `bref::iwrite(value, size)` for consistency of the interface.  Your pre v2 code will likely compile without complaint but result in unspecified behavior.  When upgrading legacy code consider temporarily renaming `write` and carefully transposing these arguments utilizing the resulting compiler errors to be sure you have updated each one.  The other ubiquitous change is `bref::read<>`, but this will solicit compiler errors already**

v3.0.0 - The largest change to the interface was the renaming of the bitstrm iterator to {mutable_,const_}bint_itr rewriting it without using the boost iterator template thus isolating boost inclusion to the unit tests only.
* additional unittests were added
* copy/match/popcount now work directly at register level (in place of iread/iwrite implementation)
* endianness implementation discarded -- never used this, better abandon than have untested code
* re-typed bref buffer to reg
* changed increment on bref::iclz to move onto 1
* printing isolated (iostream inclusion) to print.hpp
* alloced_bref allocate() renamed to resize() to better match std::vector interface
* a couple of more examples have been added and others corrected

## Motivation

The primary motivation for compression is to improve performance and or permit problems that would otherwise exceed memory capacity to be solved.  Improvement of performance may often be achieved by reducing transit to and from from slower memory sources and deposits.  Additional mechanisms for performance improvement include, increasing the information density, particularly in conjunction with improvements in data locality to win the optimization either at the CPU memory bus level or at the CPU persistent memory/rack/switch level. Further improvements in data density may permit advanced (and typically larger) or additional data structures to reside in the same or smaller memory footprint as their less performant counterparts. 

The Bitstrm library may be used directly or to an built up a codec or binary data structure that is not limited to 2^x byte boundaries.

## Call for Action in 2025

Even 10 years ago it was trivial to improve performance with data compression, i.e. improve effective cache efficiency to measurable overall performance gains by cutting the register and byte up to its bits.  Today this approach is a lot less certain.  Perhaps this makes sense, as byte addressing processors are likely more immmune to wasting say 4.3 bits per byte on average as the overall memory and memory busses grow larger and quicker.  Undoubtedly there are still applications where compression supports solutions by reducing the level of memory storage of file i/o, inter process and inter network communication.  I have not performed much by way of recently on simulation or performance instrumentation but if anyone is interested I would gladly consider your results, opinions and/or pull requests. 

## Optimization

Effort has been made to reduce branch operations especially within bref access, but not a lot of optimization or tuning otherwise.  With just a cursory look (commit dd3a315a05eac7f4a97d5747a114436d18d4eda3, Intel(R) Core(TM) i5-7360U CPU @ 2.30GHz on VB6.0 fedora:32) I am seeing a 1.02-6 times penalty for using the bitstrm library. Earlier commits of this library run on legacy hardware were capable of performance acceleration with only a little fiddling of the challange size.  Comparison of bitstrm against 2 byte words results (instead of 8 byte) yeilds nearly identical results indicating that main RAM latency is not even being significanlty driven on new(er) CPUs.

## Implementation

Implemented primarily as header only library `-std=c++23`.  It would be reasonable to maintain a `c++11` version if desired.

This code offers lightweight compilation (e.g. avoiding weighty `std::ostream` for print comes in with optionally such as with `#include "/bitstrm/print.hpp"` inclusion ).  Note that I've included a few references to `boost_1_57`.  They may be omitted when you pare down the library by removing the iterator and unittest portions.  Optional example code and boost style unit testing can be built with `cmake`.

## Features

* adaptable to different hardware, reg is defined as int64_t and it can be easily modified to be your native signed register
* pointer and iterator _lightweight accessor_ behavior (i.e. can work directly in std algorithms)
* unsigned and two's compliment conversinon an arbitrary k-bit, k:[0,register) integers
* runlengh specified(rls) separates the magnitude from the mantissa, thus {'', '0', '00', ...} can store different values instead of all zero.  (This has been applies to signed values as well).
* run lenght encoded, given a packet size encode (signed or unsigned) value expanding into other packets as necessary
* count leading zeros(clz), mismatch/equal, popcount for bitstrm

## Architecture

### Basic
`reg` and `ureg`, defined as `int64_t` and `uint64_t` respectively, serve as the upper integer size or internal working word.  Large words are generally preferred as they require less fetching and word boundry stitching assuming similar performance among word size. 

The `bref` class codecs to/from the bitstrm `reg`/`ureg` (e.g. with extent [0,64) bits) with the default behavior to be a pointer to an individual bit, and with methods for pulling that and subsequent bits to either a signed or unsigned integer value.

```
// example, storage and retrieval of 3 bit integer
alloced_bref example_buf(c_at_least_3_and_internally_stored_on_full_64_bit_boundry);
bref begin = example_buf;
example_buf.iwrite(-4, min_bits(-4));     // min_bits returns the minimum 2's complement size necessary
                                          // write 3 bits encoding -4 to example_buf while advancing
                                          // now, encoded as a single signed integer, [begin, end) -> -4
bref end = example_buf;
assert(begin.read<reg>(end-begin) == -4); // recover the three bit value (as reg or signed), note don't advance begin
example_buf.reset();                      // reset bref::example_buf to beginning of bitstrm
assert(begin == example_buf);

```
### Externalization of Magnitude

There are many methods of recording numbers.  For integers the common is a binary or 2's compliment positional radix system of fixed width. Think of bitstrm as a superset of the traditional view as it is not limited to fixed widths of k : {8, 16, 32, …, max_reg_ister_size} with numbers falling on falling on (1, 2, 4, 8}-byte boundaries.  Additionally, bitstrm supports a second _run length specified_ (_rls_) format for whole numbers which maximally utilize externalized magnitude.  The magnitude(value) or k-bits is defined as (min_bits(value + 1) - 1) thus allowing for the coding of values on top of the basis 2^_k_ -1.  In contrast to positional radix _rls_ padding alters the base by increasing k, thus more efficiently utilizes bit state (e.g. '0b' != '0b0' != '0b00' and instead corresponds to values {0, 1, 3} respectively).  _Rls_ may be deployed  where magnitude is known. 

Bitstrm is oriented about the exploit that numbers may be decomposed into magnitude and remainder loosening reliance of the fixed width, hence permitting packing of arbitrary widths into a contiguous streams of bits.  When the magnitude is know, bound or if there exists a mechanism to store, that part of the value need not be stored directly within the number.  Additionally such a magnitude it may serve additional purposes of storing the extent of the mantissa value and potentially the extent or offset within a larger a set of numbers. Thus decomposition serves as a powerful compressive, indexing or analytic technique providing a mechanism useful for multiple packed values with little additional processing or code complexity.

### Rational for `0` Width Integer:

With binary or 2's compliment radix, bound magnitude numbers are often useful in arrays where each element can be represented in _k_-bits.  Allowing _k_ = _{0,1,2,3,…}_, note that absolute  _x_ is strictly less than the bounding magnitude, i.e. 2^_k_ > |_x_| (as it forms an excluding upper bound), for _k_ = 0 and since 2^_0_ -> 1,  only absolute whole number less than _1_ must be _0_.


Intuitively considering known magnitude numbers of _k_—bits, allow for the most significant bit to be i:{null, 0, 1, 2, ..} correspond to bsize:{0, 1, 2, 3, ...} respectively.  Clearly i= 0, 2^0 -> 1, but what of i = null or 2^null?  This should be less than 1 by sequential order and non overlapping with +/- {} or any other representative number hence 2^null <=> 0 value.  Note that rls encoding of 0b0 does not imply the 0 value but rather the (1 << i) - 1 + 0b0 <=> 1 value, likewise 0b00, 0b000, ... would not represent 0, rather they can represent other values! see: `bref.hpp /run length/`

## State

I am comfortable with `gcc` and `clang` compile on `x86` platform, and be very grateful for suggestions to root out any language unspecified behavior and/or unnecessary branching or other optimization.  

## Examples

### what can you do with up to 3 bits?
```
  alloced_bref alloc0(3);
  bref p0 = alloc0;             // make some 'pointers'
  bref p1 = p0;
  assert(p0 == p1);             // pointers align
  p0.iwrite(6, 3);              // write small unsigned value 6, incr p0
  assert(p0 > p1);              // p0 past p1
  assert(p0 - p1 == 3);         // by 3 bits
  swap(p0, p1);                 // lets reorder pointers
  assert(p0.read<ureg>(3) == 6); // recover value
  alloced_bref alloc1(60);      // 20x alloc is actually the same, underneath a uint64
  assert(alloc0 != alloc1);     // alloc1 wrt alloc0 cannot coincide
  bref p2 = alloc1;
  bref p3 = copy(p0, p1, p2);    // copy the underlying bits
  assert(p3 - p2 == 3);          // (all three of them)
  assert(p0.read<ureg>(3)
	 == p2.read<ureg>(3));   // the value got copied
  // note that with three bits one can store {-4, -3, -2, -1, 0, 1, 2, 3}
  p0.write(-2, 3);               // store -2 into alloc0
  assert(p0.read<reg>(3) == -2);  // writing keeps sign, reading must specify fmt
  assert(p0.read<reg>(3)
	 == p2.read<reg>(3));    // what gives?! while -2 != 6,
  // -2 == reg(6U) when your register is 3-bit

  // back to the world of whole numbers,
  // of the subset of those of size is 3 bits width,
  // whats the maximum value you can store?
  p0.write(7 /*or 0b111*/, 3);
  assert(bref(p0).iread_rls<ureg>(3) == 14); // kind of strange?!
  p2.write(0 /*or 0b000*/, 3);               // here's a hint
  assert(bref(p2).iread_rls<ureg>(3) == 7);  // the minimum value is not zero but 7
  assert(bref(p2).iread_rls<ureg>(0) == 0);  // and, zero takes *0* bits to store,  or {0}
  assert(bref(p0).iread_rls<ureg>(2) == 6 && bref(p2).iread_rls<ureg>(2) == 3);  // the range for 2 bit rls
  assert(bref(p0).iread_rls<ureg>(1) == 2 && bref(p2).iread_rls<ureg>(1) == 1);  // the range for 1 bit rls
  // or {0}{1, 2}{3, 4, 5, 6}{7, 8, 9, 10, 11, 12, 13, 14},
  // for ranges of 0, 1, 2, and 3 bits respectively
  
```
### bint_itr example

```
  alloced_bref p0(1 * 13);          // allocate for 1 13 bit value
  mutable_bint_itr<ureg> a(p0, 13);
  auto  e(a + 1); 

  *a = 40;                          // [0] write that value
                                    // [1] assign and dereference value
                                    // [2] walk and dereference value again
  cout << *a << endl;               // > 40
  cout << (*a=42) << endl;          // > 42
  cout << (*(--(++a)))  << endl;    // > 42
  const_bint_itr<ureg> c = a;
  cout << *c << endl;               // > 42
  
  ++a;
  const_bint_itr<ureg> end_by_bref(p0 + 13*1, 13);
  assert(a == e && a == end_by_bref);

```

### bit_int_iter Create and Check Random 10 Bit Complete Table 
Create two tables @ buf and @ buf2, where buf contains complete set of 10 bit values in randomized sequence. The buf2 table is a bit vector used to verify that the 10 bit table is whole and complete.

```

  std::random_device rd;
  std::mt19937 g(rd());
  
  alloced_bref buf(10*1024);               // allocate space for 1024 10 bit values
  mutable_bint_itr<ureg> beg(buf, 10);   
  auto end(beg + 1024);
  assert((end - beg) == 1024);            
                                           // assign {0, 1, 2, ..., 1023}
  generate(beg, end, [](){ static unsigned i; return i++; });
  shuffle(beg, end, g);

  alloced_bref buf2(1024);
  buf2.zero();
  for_each(beg, end, [&buf2](ureg val){
    assert((buf2 + val).iread<ureg>(1) == 0);
    (buf2 + val).iwrite(1, 1);
  });

  // passing each of the above assertions (1024) of them,
  // have set each and every bit exactly once, hence completeness
  // proven to original assignment

  assert(popcount(buf2, buf2 + 1024) == 1024); // double check all 1's

```


