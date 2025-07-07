# bitstrm/unittest

ctest unit tests that generally follow organization of the library

```
-*- mode: compilation; default-directory: "~/git/bitstrm/build/" -*-
Compilation started at Sun Jul  6 11:40:18

cd ~/git/bitstrm/build; make test
Running tests...
Test project /home/chris/git/bitstrm/build
    Start 1: bitstrm_unittest
1/6 Test #1: bitstrm_unittest .................   Passed    0.15 sec
    Start 2: bref_unittest
2/6 Test #2: bref_unittest ....................   Passed    0.05 sec
    Start 3: reg_unittest
3/6 Test #3: reg_unittest .....................   Passed    0.01 sec
    Start 4: print_unittest
4/6 Test #4: print_unittest ...................   Passed    0.00 sec
    Start 5: utility_unittest
5/6 Test #5: utility_unittest .................   Passed    0.06 sec
    Start 6: bint_itr_unittest
6/6 Test #6: bint_itr_unittest ................   Passed    0.00 sec

100% tests passed, 0 tests failed out of 6

Total Test time (real) =   0.28 sec

Compilation finished at Sun Jul  6 11:40:18, duration 0.36 s
```
