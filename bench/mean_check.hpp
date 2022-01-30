#ifndef EMP_ZK_MEAN_H
#define EMP_ZK_MEAN_H

#include "utils.hpp"
#include "range_check.hpp"

void mean_check(uint64_t inp_sum, IntFp zk_inp_sum, int n, uint64_t &mean, IntFp &zk_mean, int fixed_point_shift, vector<IntFp> &zk_zero_checking) {
	// Store mean in fixed-point
	mean = inp_sum * fixed_point_shift / n;
	zk_mean = IntFp(mean, ALICE);

	// Check that: #samples * mean_times_shift <= sum * fixed_point_shift < #samples * mean_times_shift_plus_one
	uint64_t gap = inp_sum * fixed_point_shift - mean * n;
	IntFp zk_gap = zk_inp_sum * fixed_point_shift + zk_mean.negate() * n;

	// gap should be >= 0
	// gap should be < #samples
	range_check(gap, zk_gap, 0, n - 1, zk_zero_checking);
}

#endif //EMP_ZK_MEAN_H