# bitstrm/unittest

ctest unit tests that generally follow organization of the library

```
bash-3.2$ sysctl -n machdep.cpu.brand_string
Intel(R) Core(TM) i5-5250U CPU @ 1.60GHz
bash-3.2$ make all test
[ 60%] Built target bit_int_itr_unittest
[ 60%] Built target bitstrm_unittest
[ 60%] Built target bref_unittest
[ 60%] Built target reg_unittest
[ 60%] Built target utility_unittest
Running tests...
Test project /Users/chris/git/t0/t0rture/bitstrm/unittest
Start 1: bref_unittest
1/5 Test #1: bref_unittest ....................   Passed    2.72 sec
Start 2: reg_unittest
2/5 Test #2: reg_unittest .....................   Passed    0.01 sec
Start 3: utility_unittest
3/5 Test #3: utility_unittest .................   Passed    1.54 sec
Start 4: bitstrm_unittest
4/5 Test #4: bitstrm_unittest .................   Passed    0.01 sec
Start 5: bit_int_itr_unittest
5/5 Test #5: bit_int_itr_unittest .............   Passed    3.38 sec

100% tests passed, 0 tests failed out of 5

Total Test time (real) =   7.68 sec
```