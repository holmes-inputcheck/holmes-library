#ifndef EMP_ZK_RANGE_CHECK_H
#define EMP_ZK_RANGE_CHECK_H

#include "utils.hpp"

void range_check(uint64_t inp, IntFp zk_inp, uint64_t B_low, uint64_t B_high, vector<IntFp> &zk_zero_checking) {
	assert(inp >= B_low);
	assert(inp <= B_high);

	// Check that each value is of the correct range
	// y[i] = (inp[i] - B_low) - \sum_{0..} (bits[j] * 2^j)
	int needed_bits_range_check = get_num_range_bits(B_low, B_high);

	IntFp tmp0, tmp1;

	tmp0 = zk_inp.negate();
	tmp0 = tmp0 + B_low;

	tmp1 = IntFp(B_high, PUBLIC);
	tmp1 = tmp1.negate();
	tmp1 = tmp1 + zk_inp;

	// Generate the binary testing bits
	// First, compute how many bits are needed.
	// And then add it to ZK_bits

	bool* bits = new bool[needed_bits_range_check * 2];

	uint64_t delta_low = inp - B_low;
	uint64_t delta_high = B_high - inp;

	for (int j = 0; j < needed_bits_range_check; j++) {
		bits[j] = delta_low & 1;
		delta_low >>= 1;

		bits[needed_bits_range_check + j] = delta_high & 1;
		delta_high >>= 1;
	}

	IntFp* zk_bits = new IntFp[needed_bits_range_check * 2];
	for (int i = 0; i < needed_bits_range_check * 2; i++) {
		zk_bits[i] = IntFp(bits[i], ALICE);
	}

	for (int i = 0; i < needed_bits_range_check * 2; i++) {
		IntFp tmp;
		tmp = zk_bits[i] * zk_bits[i];
		tmp = tmp + zk_bits[i].negate();

		zk_zero_checking.emplace_back(tmp);
	}

	uint64_t cur = 1;
	for (int j = 0; j < needed_bits_range_check; ++j) {
		tmp0 = tmp0 + zk_bits[j] * cur;
		tmp1 = tmp1 + zk_bits[needed_bits_range_check + j] * cur;

		cur <<= 1;
	}

	zk_zero_checking.emplace_back(tmp0);
	zk_zero_checking.emplace_back(tmp1);

	delete[] bits;
	delete[] zk_bits;
}

#endif //EMP_ZK_RANGE_CHECK_H
