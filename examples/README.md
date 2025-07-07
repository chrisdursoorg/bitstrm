# bitstrm/examples


## system

Just a call to `lscpu`, a build with Fedora on an older model Dell Latitude laptop.

```
[chris@fedora examples]$ ./system 
Architecture:                x86_64
  CPU op-mode(s):            32-bit, 64-bit
  Address sizes:             39 bits physical, 48 bits virtual
  Byte Order:                Little Endian
CPU(s):                      8
  On-line CPU(s) list:       0-7
Vendor ID:                   GenuineIntel
  Model name:                11th Gen Intel(R) Core(TM) i7-1185G7 @ 3.00GHz
    CPU family:              6
    Model:                   140
    Thread(s) per core:      2
    Core(s) per socket:      4
    Socket(s):               1
    Stepping:                1
    CPU(s) scaling MHz:      33%
    CPU max MHz:             4800.0000
    CPU min MHz:             400.0000
    BogoMIPS:                3609.60
    Flags:                   fpu vme de pse tsc msr pae mce cx8 apic sep mtrr pge mca cmov pat pse36 clflush dts acpi mmx fxsr sse sse2 ss ht tm pbe syscall nx pdpe1gb
                              rdtscp lm constant_tsc art arch_perfmon pebs bts rep_good nopl xtopology nonstop_tsc cpuid aperfmperf tsc_known_freq pni pclmulqdq dtes64
                              monitor ds_cpl vmx smx est tm2 ssse3 sdbg fma cx16 xtpr pdcm pcid sse4_1 sse4_2 x2apic movbe popcnt tsc_deadline_timer aes xsave avx f16c
                              rdrand lahf_lm abm 3dnowprefetch cpuid_fault epb cat_l2 cdp_l2 ssbd ibrs ibpb stibp ibrs_enhanced tpr_shadow flexpriority ept vpid ept_ad
                              fsgsbase tsc_adjust bmi1 avx2 smep bmi2 erms invpcid rdt_a avx512f avx512dq rdseed adx smap avx512ifma clflushopt clwb intel_pt avx512cd 
                             sha_ni avx512bw avx512vl xsaveopt xsavec xgetbv1 xsaves split_lock_detect user_shstk dtherm ida arat pln pts hwp hwp_notify hwp_act_window
                              hwp_epp hwp_pkg_req vnmi avx512vbmi umip pku ospke avx512_vbmi2 gfni vaes vpclmulqdq avx512_vnni avx512_bitalg avx512_vpopcntdq rdpid mov
                             diri movdir64b fsrm avx512_vp2intersect md_clear ibt flush_l1d arch_capabilities
Virtualization features:     
  Virtualization:            VT-x
Caches (sum of all):         
  L1d:                       192 KiB (4 instances)
  L1i:                       128 KiB (4 instances)
  L2:                        5 MiB (4 instances)
  L3:                        12 MiB (1 instance)
NUMA:                        
  NUMA node(s):              1
  NUMA node0 CPU(s):         0-7
Vulnerabilities:             
  Gather data sampling:      Mitigation; Microcode
  Ghostwrite:                Not affected
  Indirect target selection: Mitigation; Aligned branch/return thunks
  Itlb multihit:             Not affected
  L1tf:                      Not affected
  Mds:                       Not affected
  Meltdown:                  Not affected
  Mmio stale data:           Not affected
  Reg file data sampling:    Not affected
  Retbleed:                  Not affected
  Spec rstack overflow:      Not affected
  Spec store bypass:         Mitigation; Speculative Store Bypass disabled via prctl
  Spectre v1:                Mitigation; usercopy/swapgs barriers and __user pointer sanitization
  Spectre v2:                Mitigation; Enhanced / Automatic IBRS; IBPB conditional; PBRSB-eIBRS SW sequence; BHI SW loop, KVM SW loop
  Srbds:                     Not affected
  Tsx async abort:           Not affected
```

## example

```
[chris@fedora examples]$ ./example 
Simple example where we we will store and then recall some binary values.
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
total_bits: 468 bytes: 58 bits/number: 14.625 savings: 77.3438%
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

## sort_perf and sort_10_bit

Sorts of a 10 bit unsigned and 10 bit signed randomly generated tables and contrast the performance against primatives (`uint64, int64, uint16, int16` etc.).  The below timings are typical with the bint_itr taking about 4-5x as long as thier native counterparts.  This is largely attributable to their being a referece_wrapper object that is being created multiple times in the sort comparison and swapping process in addition to the bit manipulation necessary for bitstrm coding/decoding of integers.

```
./sort_perf 
TIMER START [0] - sort 8388608 unsigned random elements of size 10 bits
TIMER START [1] - created 8388608 random elements storing as ureg, unsigned short and 10 bit ureg
TIMER END [1] created 8388608 random elements storing as ureg, unsigned short and 10 bit ureg -  completed @ 15:49:03 elapsed time: 0.105094s
TIMER START [2] - sorting as ureg
TIMER END [2] sorting as ureg -  completed @ 15:49:04 elapsed time: 0.270582s @3.22559e-08 s/cycle freq: 3.10021e+07/s
TIMER START [3] - sorting as unsigned short
TIMER END [3] sorting as unsigned short -  completed @ 15:49:04 elapsed time: 0.258885s @3.08615e-08 s/cycle freq: 3.24029e+07/s
TIMER START [4] - sorting as 10 bit integer
TIMER END [4] sorting as 10 bit integer -  completed @ 15:49:05 elapsed time: 0.892742s @1.06423e-07 s/cycle freq: 9.39645e+06/s
TIMER END [0] sort 8388608 unsigned random elements of size 10 bits -  completed @ 15:49:05 elapsed time: 1.56517s
```



```
[chris@fedora examples]$ ./sort_10_bit 
TIMER START [0] - sort 1048576 signed random elements of size 10 bits
TIMER START [1] - created 1048576 random elements storing as reg, short and 10 bit reg
TIMER END [1] created 1048576 random elements storing as reg, short and 10 bit reg -  completed @ 15:29:34 elapsed time: 0.0164031s
TIMER START [2] - sorting as reg
TIMER END [2] sorting as reg -  completed @ 15:29:34 elapsed time: 0.039872s @3.80249e-08 s/cycle freq: 2.62986e+07/s
TIMER START [3] - sorting as signed short
TIMER END [3] sorting as signed short -  completed @ 15:29:34 elapsed time: 0.0359417s @3.42767e-08 s/cycle freq: 2.91744e+07/s
TIMER START [4] - sorting as 10 bit signed integer
TIMER END [4] sorting as 10 bit signed integer -  completed @ 15:29:34 elapsed time: 0.112752s @1.07529e-07 s/cycle freq: 9.29984e+06/s
TIMER END [0] sort 1048576 signed random elements of size 10 bits -  completed @ 15:29:34 elapsed time: 0.218254s
```

## cycle_test

The cycle test creates a random single link grah of arbitrary size.  The program then fully traverses the graph in order to pick out the smallest and largest cycle within it along with the count of how many times that frequency occurs.  As with the sort 10 it times the performance of `vector<min_size_type_for_extent>` and `bit_int_itr`.  Note that the `bitstrm<min_size_for_extent>` implementation is quicker than the native format.

```
[chris@fedora examples]$ ./cycle_test 1234567
This simple algorithm to obtains the count of the very
smallest cycle, and the very largest cycle of a psudo-random
generated singely linked graph in a way that that requires only
a couple bits per node of information for current traversal state.


graph size: 1234567
fits in 3.09061 mbytes with bitstrm or in 4.7095 mbytes using an array of 4-bytes (bitstrm saves you 34.375%)
TIMER START [0] - assign1234567 verticies single edges to themselves
TIMER END [0] assign1234567 verticies single edges to themselves -  completed @ 15:33:07 elapsed time: 0.00111661s
TIMER START [1] - shuffle edges randomly among verticies
TIMER END [1] shuffle edges randomly among verticies -  completed @ 15:33:07 elapsed time: 0.049832s
TIMER START [2] - orphan nodes forming nontrivial graph
TIMER END [2] orphan nodes forming nontrivial graph -  completed @ 15:33:07 elapsed time: 1.043e-06s
TIMER START [3] - copy to bitstrm
TIMER END [3] copy to bitstrm -  completed @ 15:33:07 elapsed time: 0.0024255s
TIMER START [4] - determine cycles on std vector
TIMER END [4] determine cycles on std vector -  completed @ 15:33:07 elapsed time: 0.0484241s
TIMER START [5] - determine cycles on bitstrm vector
TIMER END [5] determine cycles on bitstrm vector -  completed @ 15:33:07 elapsed time: 0.0420869s
std min: λ: 1 @2 max: λ: 94951 @1

```

## consolidate_perf

```
[chris@fedora examples]$ ./consolidate_perf 

Given an input stream of integers with a bounding maximal extent,
consolidate the values into a discovered smaller stream.  Natuarlly
bitstrm may take less space but how much a performance hit or 
gain will result?

example data:
0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, ...
and
-8, -7, -6, -5, -4, -3, -2, -1, 0, 1, 2, 3, 4, 5, 6, 7, -8, -7, -6, -5, -4, -3, -2, -1, 0, 1, 2, 3, 4, 5, 6, 7, ...

Test 64 -> 8 bits v 64 -> 4 bits test_size: 67108864

TIMER START [0] - unsigned copy to 8 bits
TIMER END [0] unsigned copy to 8 bits -  completed @ 11:03:30 elapsed time: 0.0351001s
TIMER START [1] - unsigned copy to 4 bits
TIMER END [1] unsigned copy to 4 bits -  completed @ 11:03:30 elapsed time: 0.0869291s
TIMER START [2] - unsigned check of 8 bits
TIMER END [2] unsigned check of 8 bits -  completed @ 11:03:30 elapsed time: 0.0273965s
TIMER START [3] - unsigned check of 4 bits
TIMER END [3] unsigned check of 4 bits -  completed @ 11:03:30 elapsed time: 0.0906227s
TIMER START [4] - signed copy to 8 bits
TIMER END [4] signed copy to 8 bits -  completed @ 11:03:30 elapsed time: 0.0351246s
TIMER START [5] - signed copy to 4 bits
TIMER END [5] signed copy to 4 bits -  completed @ 11:03:30 elapsed time: 0.0865842s
TIMER START [6] - signed check of 8 bits
TIMER END [6] signed check of 8 bits -  completed @ 11:03:31 elapsed time: 0.0348462s
TIMER START [7] - signed check of 4 bits
TIMER END [7] signed check of 4 bits -  completed @ 11:03:31 elapsed time: 0.0898116s
```

## interior_block_delete

Given an allocated bitstrm have deleted sections consolidate remaining sections such that data may be appended to the end

```
[chris@fedora examples]$ ./interior_block_delete 
TIMER START [0] - shift_left and check signed 8 bit
with 112 shifts moved a total of 34750119040 bits 4142.54 m bytes.
TIMER END [0] shift_left and check signed 8 bit -  completed @ 11:06:18 elapsed time: 3.04769s
TIMER START [1] - shift_left and check unsigned 8 bit
with 112 shifts moved a total of 34750119040 bits 4142.54 m bytes.
TIMER END [1] shift_left and check unsigned 8 bit -  completed @ 11:06:22 elapsed time: 3.31775s
TIMER START [2] - shift_left and check signed 4 bit
with 112 shifts moved a total of 17375059520 bits 2071.27 m bytes.
TIMER END [2] shift_left and check signed 4 bit -  completed @ 11:06:28 elapsed time: 5.11481s
TIMER START [3] - shift_left and check unsigned 4 bit
with 112 shifts moved a total of 17375059520 bits 2071.27 m bytes.
TIMER END [3] shift_left and check unsigned 4 bit -  completed @ 11:06:33 elapsed time: 4.4982s

```
