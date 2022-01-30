#ifndef EMP_ZK_VARIANCE_CHECK_HPP
#define EMP_ZK_VARIANCE_CHECK_HPP

#include "utils.hpp"
#include "range_check.hpp"

void variance_check(uint64_t inp_sum, IntFp zk_sum, uint64_t inp_squared_sum, IntFp zk_inp_squared_sum, int n, uint64_t &variance, IntFp &zk_variance, int fixed_point_shift, vector<IntFp> &zk_zero_checking) {
	// compute the variance
	variance = (inp_squared_sum * n - inp_sum * inp_sum) * fixed_point_shift / n / n;
	zk_variance = IntFp(variance, ALICE);

	uint64_t gap = inp_squared_sum * n * fixed_point_shift - inp_sum * inp_sum * fixed_point_shift - variance * n * n;
	IntFp zk_gap = zk_inp_squared_sum * n * fixed_point_shift + zk_sum.negate() * zk_sum * fixed_point_shift + zk_variance.negate() * n * n;

	// gap should be >= 0
	// gap should be < #samples * #samples
	range_check(gap, zk_gap, 0, (uint64_t)n * n - 1, zk_zero_checking);
}

#endif //EMP_ZK_VARIANCE_CHECK_HPP
