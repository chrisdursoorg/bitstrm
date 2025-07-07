// bitstrm.hpp
//
//

#ifndef BITSTRM_HPP
#define BITSTRM_HPP

// A bitstrm is a sequence of bits of arbitrary length that is backed by a
// contiguous array of ureg (unsigned cpu optimum 'register' size integer as
// defined in reg.hpp).  Experiments have allowed for packing into smaller
// units (i.e. char) with endian transformations.  Though time costs of endian
// adjustment were unnoticed the added complexity/maintenance versus application
// benefit resulted in its abandonment.
//
// The bitstrm is optimized for the storage and retrival of short signed and
// unsigned integers (relative to and a subset of reg/ureg ) in a variety of
// formats where the majority operations both occur within one ureg boundry and
// sequentially. The methods constituting the majority of this library reside in
// the bref a lightweight 'pointer like object' and the only slightly more
// encombered bit_int_iter for sequences of fixed size and signed/unsigned
// integers.
//

#include <utility>
#include <bitstrm/bref.hpp>

namespace bitstrm {

  // copy
  //
  // defined for ranges that are non overlapping!
  //
  // copies the bits from [begin, end) into the range beginning with
  // destination returns an bref of the end of the destination range
  //
  bref copy(bref begin, bref end, bref destination);

  // mismatch
  //
  // Returns a pair of iterators to the first mismatching of elements
  // from [bein, end) and a range starting from target
  std::pair<bref, bref>  mismatch(bref begin, bref end, bref target);

  // equal
  // 
  // for [begin, end) return true if equivalent to [second, target +
  // end - begin)
  bool equal(bref begin, bref end, bref target);

  // clz
  //
  // leading zero run, for [beg, end),
  // return a bref at the first hi bit in range or else end
  bref clz(bref beg, bref end);
  
  // popcount
  //
  // population count of ones in [beg, end), O(ones) complexity
  ureg popcount(bref beg, bref end);

  #include <bitstrm/bitstrm_impl.hpp>
  
} // namespace bitstrm


#endif // def'd BITSTRM_HPP
