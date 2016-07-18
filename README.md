# About bitstrm 
Library supporting streams of arbitrary signed and unsigned integers in contagious blocks

Bitstrm is a C++ template library to write strings of bits of arbitrary length representing either signed or unsigned values as a precursor to general compression or packed array work.  In addition to fixed sized read/write operations for signed unsigned values a run length leading zeros count operator is included.

The basis for the bit operation is a 64 bit register (though that may be redefined) and it should be relatively simple to switch between big-endian and little-endian byte positioning and allowing for sub register storage/retrieval.

In the examples &std::vector<char>::front() is used for the underlying data.  If you are to use this library, you would likely be utilizing an alternate memory allocation and the interfaces should hold for, stack, heap, or pool allocation.

### Documentation

Hopefully reasonably well documented in code.

unittest/* contains tests and examples of all the functions in the library including encode/decode operators, sizing and addressing arithmetic, copy and equivalence operators, and iterators

performance/ contains encoding decoding stats as well as benchmark against register, and short integer values in std::sort

### Example 

Check out [example.cpp](https://github.com/chrisdursoorg/bitstrm/blob/master/example.cpp), at just over a 100 lines of code its a little bit much to paste here.  [example.cpp](https://github.com/chrisdursoorg/bitstrm/blob/master/example.cpp) encodes 32 largely small integers ranging [0,63 bits] by coding each integer as [[_Size_]<_Value_><_Control_>] where _Size_ is a 6 bit integer _Size_ range [0..63], _Value_ is the value encoded as unsigned integer of _Size_ bits and _Control_ is a boolean indicating that the next optional _Size_ field is inlined and must be reset for subsequent _Value_s.  

The coding saves space by keeping a running count of how many bits are wasted.  It saves a little more space than just prefixing the size before each value as it illustrates the flexibility of bitstrm.  When the cumulatively wasted is greater than saving a _Size_ the _Control_ bit goes high indicating a reseting _Size_ and the cumulative waste.  In this example the codec saves 77% when contrasting with the standard 64 bit representation.

This example is meant to be simple and not really practical.  The codec could for instance make more passes and determine the best places to reset the _Size_. And one can note that whenever _Size_ is reset, the most significant bit of the subsequent value is implied, hence we redundantly save this one bit.  Finally, maximally 63 bit integers are supported as _Size_ is in range of [0,63].  Since a _Size_ 0 bitstrm integer is well defined (as zero) we would have to use a 7 bit _Size_ field to address 64 bit values, and then what would we do with all those extra range of [65,126]?

```
~/git/build/example
...
Simple example where we are going to store and then recall some binary values.
The values will generally be near 0 and will all be unsigned

This codec is made mostly for simplicity, rather than utility.
we could for instance recognize that first Value 1st bit is
redundant with Size magnitude and do a better job deciding where
to have an extra pad 0 bit or a new Size block.

Decode: [[Size]<Value><Control>], if control is 1 read [Size], [Size] is 6 bits
Code:   [[Size]<Value><Control>], adopt a Size to hold Value, retain Size until until
1) Size cannot hold Value (not enough bits), or 2) total wasted bits (preceeding zeros)
is greater than 6

example data:
12,
0,
1,
2,
3,
4,
5,
1234567,
922338962904924173,
65,
82,
23,
11,
1,
1,
1,
890,
54,
0,
0,
0,
0,
17,
232902332,
89,
2398,
12,
12,
12,
558,
235,
238923,

total_bits: 468 bytes: 59 bits/number: 14.625 savings: 76.9531%
verify: 468 bits used equals: 468 bits reserved
restore: 12 matches: 12
restore: 0 matches: 0
restore: 1 matches: 1
restore: 2 matches: 2
restore: 3 matches: 3
restore: 4 matches: 4
restore: 5 matches: 5
restore: 1234567 matches: 1234567
restore: 922338962904924173 matches: 922338962904924173
restore: 65 matches: 65
restore: 82 matches: 82
restore: 23 matches: 23
restore: 11 matches: 11
restore: 1 matches: 1
restore: 1 matches: 1
restore: 1 matches: 1
restore: 890 matches: 890
restore: 54 matches: 54
restore: 0 matches: 0
restore: 0 matches: 0
restore: 0 matches: 0
restore: 0 matches: 0
restore: 17 matches: 17
restore: 232902332 matches: 232902332
restore: 89 matches: 89
restore: 2398 matches: 2398
restore: 12 matches: 12
restore: 12 matches: 12
restore: 12 matches: 12
restore: 558 matches: 558
restore: 235 matches: 235
restore: 238923 matches: 238923
```
---

### Building and Unittests

```
bash-3.2$ # https://github.com/chrisdursoorg/bitstrm
bash-3.2$ # MacBook Air (13-inch, Early 2015)
bash-3.2$ sysctl -n machdep.cpu.brand_string
Intel(R) Core(TM) i5-5250U CPU @ 1.60GHz
bash-3.2$ git clone git@github.com:chrisdursoorg/bitstrm.git bitstrm
bash-3.2$ mkdir build; cd build
bash-3.2$ cmake -DCMAKE_BUILD_TYPE='Release' ../bitstrm
defaulting to Release mode, do you want -DCMAKE_BUILD_TYPE='Debug' ?
Release build.
...
-- Build files have been written to: /Users/chris/git/build2
bash-3.2$ make
Scanning dependencies of target bitstrm
[  8%] Building CXX object bitstrm/CMakeFiles/bitstrm.dir/bitstrm.cpp.o
[ 16%] Linking CXX static library libbitstrm.a
...
[100%] Built target performnance_tests
# run the unit tests, otherwise you could just manually ./unittest/ ...
bash-3.2$ make test
Running tests...
Test project /Users/chris/git/build2
    Start 1: reg_unittest
1/4 Test #1: reg_unittest .....................   Passed    0.04 sec
    Start 2: utility_unittest
2/4 Test #2: utility_unittest .................   Passed    0.27 sec
    Start 3: bitstrm_unittest
3/4 Test #3: bitstrm_unittest .................   Passed    1.45 sec
    Start 4: bit_int_itr_unittest
4/4 Test #4: bit_int_itr_unittest .............   Passed    0.25 sec

100% tests passed, 0 tests failed out of 4

Total Test time (real) =   2.02 sec
```

### Performance 

In the sort example of the performance test, you see a modest savings of about 6% reducing the unnecessary large 64-register to a more reasonable 16-bit short.  Futher reducinng the __register__ to 10 bits through the bitstream library actually cost you about 3x performance (see below, this is because you are having to do a lot of bit manipulation to decode and code each value and carry left the signed bit, often stiching together 2 registers).  This I feel is a good example as a significant goal of compression is to reduce CPU time and this example with its random-access and copious writes would do about as poorly as one can do.  Note also that this test is done on a laptop, and a single thread so one must take it with that perspective.

A contrasting technology [https://github.com/lemire/SIMDCompressionAndIntersection](lemire/SIMDCompressionAndIntersection) claims 8x improvement in speed with X86 SIMD instructions, hence __this__ library would only be useful when you could speed up the process algorithmically by a factor of 24.  You might just be able to do that, as writing to an arbitrary (sub 64 bit) number of bits as though they were 64 bits in random access abstraction allows for the algorithm writer to pack contextually relavent information greatly.  Given a ballpark ~500x boost between register speed and RAM speed (then also there is bandwith saturation and CPU memory starvation), the grain boundaries of SIMD (which has byte boundries) and finally the intrinsic random access nature of this library (you can't easily sort with SIMD) then you may well be able to achieve superior algorithms from the speed perspective

```
bash-3.2$ ./performance/performnance_tests
In a ~128 m data point array, with a couple of initial gaps, copy array left over gap, with bitstrm copy
TIMER START [0] - copy 128m
enumeration of 128m data points complete
TIMER START [1] - copy itself
TIMER END [1] copy itself -  completed @ 16:17:41 elapsed time: 0.742986s @5.5357e-09 s/cycle freq: 1.80646e+08/s
128m data points bitwise copied
successful checking 128m data points
TIMER END [0] copy 128m -  completed @ 16:17:41 elapsed time: 2.83548s


Performance test of 1 B signextend operations from 1K (9 bit depth integers) pool
TIMER START [2] - 1G signext test
created a sample of 1K data points
TIMER START [3] - 1B signext
TIMER END [3] 1B signext -  completed @ 16:17:42 elapsed time: 0.942324s @8.77608e-10 s/cycle freq: 1.13946e+09/s
TIMER END [2] 1G signext test -  completed @ 16:17:42 elapsed time: 0.942412s


TIMER START [4] - sort 8388608 unsigned random elements of size 10 bits
TIMER START [5] - created 8388608 random elements storing as ureg, unsigned short and 10 bit ureg
TIMER END [5] created 8388608 random elements storing as ureg, unsigned short and 10 bit ureg -  completed @ 16:17:42 elapsed time: 0.087549s
TIMER START [6] - sorting as ureg
TIMER END [6] sorting as ureg -  completed @ 16:17:43 elapsed time: 0.365085s @4.35215e-08 s/cycle freq: 2.29771e+07/s
TIMER START [7] - sorting as unsigned short
TIMER END [7] sorting as unsigned short -  completed @ 16:17:43 elapsed time: 0.355295s @4.23545e-08 s/cycle freq: 2.36103e+07/s
TIMER START [8] - sorting as 10 bit integer
TIMER END [8] sorting as 10 bit integer -  completed @ 16:17:44 elapsed time: 1.098s @1.30892e-07 s/cycle freq: 7.63986e+06/s
TIMER END [4] sort 8388608 unsigned random elements of size 10 bits -  completed @ 16:17:44 elapsed time: 1.96449s


TIMER START [9] - sort 8388608 signed random elements of size 10 bits
TIMER START [10] - created 8388608 random elements storing as reg, short and 10 bit reg
TIMER END [10] created 8388608 random elements storing as reg, short and 10 bit reg -  completed @ 16:17:44 elapsed time: 0.084838s
TIMER START [11] - sorting as reg
TIMER END [11] sorting as reg -  completed @ 16:17:45 elapsed time: 0.370344s @4.41484e-08 s/cycle freq: 2.26509e+07/s
TIMER START [12] - sorting as signed short
TIMER END [12] sorting as signed short -  completed @ 16:17:45 elapsed time: 0.348954s @4.15986e-08 s/cycle freq: 2.40393e+07/s
TIMER START [13] - sorting as 10 bit signed integer
TIMER END [13] sorting as 10 bit signed integer -  completed @ 16:17:46 elapsed time: 1.08963s @1.29894e-07 s/cycle freq: 7.69861e+06/s
TIMER END [9]  -  completed @ 16:17:46 elapsed time: 1.92156s
```

