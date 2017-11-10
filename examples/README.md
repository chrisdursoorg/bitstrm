# bitstrm/examples

## sort_10_bit

The following illustrates a sort of a 10 bit table and contrast the performance against simple primatives (`vector<uint64_t>` etc.).  The below timings are typical with the bit_int_itr taking about 4-5x as long as thier native counterparts.  This is largely attributable to their being a referece_wrapper object that is being created multiple times in the sort comparison and swapping process, in addition to the bit manipulation necessary for bitstrm.

```
bash-3.2$ sysctl -n machdep.cpu.brand_string
Intel(R) Core(TM) i5-5250U CPU @ 1.60GHz
bash-3.2$ ./sort_10_bit
TIMER START [0] - sort 1048576 signed random elements of size 10 bits
TIMER START [1] - created 1048576 random elements storing as reg, short and 10 bit reg
TIMER END [1] created 1048576 random elements storing as reg, short and 10 bit reg -  completed @ 17:28:21 elapsed time: 0.011624s
TIMER START [2] - sorting as reg
TIMER END [2] sorting as reg -  completed @ 17:28:21 elapsed time: 0.051985s @4.95768e-08 s/cycle freq: 2.01707e+07/s
TIMER START [3] - sorting as signed short
TIMER END [3] sorting as signed short -  completed @ 17:28:21 elapsed time: 0.059124s @5.6385e-08 s/cycle freq: 1.77352e+07/s
TIMER START [4] - sorting as 10 bit signed integer
TIMER END [4] sorting as 10 bit signed integer -  completed @ 17:28:22 elapsed time: 0.234133s @2.23287e-07 s/cycle freq: 4.47855e+06/s
TIMER END [0] sort 1048576 signed random elements of size 10 bits -  completed @ 17:28:22 elapsed time: 0.365772s
```

## cycle_test

The cycle test creates a random single link grah of arbitrary size.  The program then fully traverses the graph in order to pick out the smallest and largest cycle within it along with the count of how many times that frequency occurs.  As with the sort 10 it times the performance of `vector<min_size_type_for_extent>` and `bit_int_itr`.  See second listing, where `bitstrm` is significantly quicker than the native format.

This first listing (slightly different than the committed code) prints out the graph in the fomat of

```[nodeid]:<links_to_list>```

Where the `^` charactor indicates the point where the cycle begins and the trailing loop size is printed `[loop_size]`. 


```
bash-3.2$ ./cycle_test 10
graph size: 10
fits in 5 bytes with bitstrm or in 10 bytes using an array of 1-bytes (bitstrm saves you 50%)
TIMER START [0] - assign 10 pointers
TIMER END [0] assign 10 pointers -  completed @ 16:38:34 elapsed time: 6e-06s
TIMER START [1] - shuffle pointers forming nontrivial graph
TIMER END [1] shuffle pointers forming nontrivial graph -  completed @ 16:38:34 elapsed time: 2.2e-05s
TIMER START [2] - orphan nodes forming nontrivial graph
TIMER END [2] orphan nodes forming nontrivial graph -  completed @ 16:38:34 elapsed time: 8e-06s
TIMER START [3] - copy to bitstrm
TIMER END [3] copy to bitstrm -  completed @ 16:38:34 elapsed time: 1.2e-05s
TIMER START [4] - determine cycles on std vector
vector:
…

TIMER END [4] determine cycles on std vector -  completed @ 16:38:34 elapsed time: 8e-05s
TIMER START [5] - determine cycles on bitstrm vector
bitstrm:

0:,7,^4,1,7[3]
1:7...
2:,^6,2[2]
3:,^3[1]
4:1...
5:0,...
6:2...
7:4...
8:5,...
9:,^9[1]
TIMER END [5] determine cycles on bitstrm vector -  completed @ 16:38:34 elapsed time: 7.4e-05s
std min: λ: 1 @2 max: λ: 3 @1
Process 19773 exited with status = 0 (0x00000000)

```
This _committed code_ listing illustrates an instance that the compressed data processes significantly less time than the native version.  As with the sort_10 graph traversal the algorithm is inherently random access.  Both native and bitstrm version do share reference_wrapper for the traversal state, however the bitstrm path utilizes a `bit_int_citr` for the graph itself which importantly *does not utilize a reference_wrapper*.  Speed improvement is attributable to the better utilization of the 3 megabyte L2 cache.  With much smaller graph or much larger graphs the native path comes back to lead performance, because the entire native graph or neither graph fits well into L2 cache.

This is only one example of perfomance improvements with bitstrm.  Other basis for improvements are addition of performance enhancing state facititated by the data compression, or due to the increasing the density of data and consequtial reduction of memory bus or network contention.

```
bash-3.2$ ./cycle_test 1234567

This simple algorithm to obtains the count of the very
smallest cycle, and the very largest cycle of a psudo-random
generated singly linked graph in a way that that requires only
a couple bits per node of information for current traversal state.

graph size: 1234567
fits in 3.09061 mbytes with bitstrm or in 4.7095 mbytes using an array of 4-bytes (bitstrm saves you 34.375%)
TIMER START [0] - assign 1234567 pointers
TIMER END [0] assign 1234567 pointers -  completed @ 17:23:52 elapsed time: 0.000598s
TIMER START [1] - shuffle pointers forming nontrivial graph
TIMER END [1] shuffle pointers forming nontrivial graph -  completed @ 17:23:53 elapsed time: 0.887355s
TIMER START [2] - orphan nodes forming nontrivial graph
TIMER END [2] orphan nodes forming nontrivial graph -  completed @ 17:23:53 elapsed time: 3e-06s
TIMER START [3] - copy to bitstrm
TIMER END [3] copy to bitstrm -  completed @ 17:23:53 elapsed time: 0.009514s
TIMER START [4] - determine cycles on std vector
TIMER END [4] determine cycles on std vector -  completed @ 17:23:54 elapsed time: 0.144699s
TIMER START [5] - determine cycles on bitstrm vector
TIMER END [5] determine cycles on bitstrm vector -  completed @ 17:23:54 elapsed time: 0.112509s
std min: λ: 2 @1 max: λ: 310034 @1
```