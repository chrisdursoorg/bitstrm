# About bitstrm 
Library supporting streams of arbitrary signed and unsigned integers in contagious blocks

Bitstrm is a C++ template library to write strings of bits of arbitrary length representing either signed or unsigned values as a precursor to general compression or packed array work.  In addition to fixed sized read/write operations for signed unsigned values a run length leading zeros count operator is included.

The basis for the bit operation is a 64 bit register (though that may be redefined) and it should be relatively simple to switch between big-endian and little-endian byte positioning and allowing for sub register storage/retrieval.

In the examples &std::vector<char>::front() is used for the underlying data.  If you are to use this library, you would likely be utilizing an alternate memory allocation and the interfaces should hold for, stack, heap, or pool allocation.

### Documentation

Hopefully reasonably well documented in code.

unittest/* contains tests and examples of all the functions in the library including encode/decode operators, sizing and addressing arithmetic, copy and equivalence operators, and iterators

performance/ contains encoding decoding stats as well as benchmark against register, and short integer values in std::sort

---

### Example Building and Tests

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

