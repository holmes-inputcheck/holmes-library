#ifndef EMP_ZK_TRIMMED_MEAN_H
#define EMP_ZK_TRIMMED_MEAN_H

#include "utils.hpp"
#include "range_check.hpp"

// This check requires the range check for the corresponding column

void trimmed_sum(uint64_t inp, IntFp zk_inp, int B_low, int B_cutoff, int B_high, uint64_t &effective_sum, IntFp &zk_effective_sum, uint64_t &effective_count, IntFp &zk_effective_count, vector<IntFp> &zk_zero_checking) {
	assert(inp >= B_low);
	assert(inp <= B_high);

	int needed_bits_special_range_check = max(get_num_range_bits(B_low, B_cutoff - 1), get_num_range_bits(B_cutoff, B_high));

	bool special_range_check_bits[needed_bits_special_range_check];
	int choice_bit;

	if(inp >= B_cutoff) {
		// prove that this point is beyond the cutoff
		int delta = inp - B_cutoff;

		for (int i = 0; i < needed_bits_special_range_check; i++) {
			special_range_check_bits[i] = delta & 1;
			delta >>= 1;
		}
		assert(delta == 0);

		choice_bit = 0;
	} else {
		// prove that this point is within the cutoff
		int delta = B_cutoff - inp;

		for(int i = 0; i < needed_bits_special_range_check; i++) {
			special_range_check_bits[i] = delta & 1;
			delta >>= 1;
		}
		assert(delta == 0);

		choice_bit = 1;
	}

	// allocate the range check bits
	IntFp zk_special_range_check_bits[needed_bits_special_range_check];
	for (int i = 0; i < needed_bits_special_range_check; i++) {
		zk_special_range_check_bits[i] = IntFp(special_range_check_bits[i], ALICE);
	}

	// perform bit testing for range check bits
	for (int i = 0; i < needed_bits_special_range_check; i++) {
		IntFp tmp;
		tmp = zk_special_range_check_bits[i] * zk_special_range_check_bits[i];
		tmp = tmp + zk_special_range_check_bits[i].negate();

		zk_zero_checking.emplace_back(tmp);
	}

	// allocate the choice bit
	IntFp zk_choice_bit = IntFp(choice_bit, ALICE);

	// perform bit testing for the choice bit
	{
		IntFp tmp;
		tmp = zk_choice_bit * zk_choice_bit;
		tmp = tmp + zk_choice_bit.negate();

		zk_zero_checking.emplace_back(tmp);
	}

	// provide two candidates for the cutoff check
	IntFp tmp0 = zk_inp + IntFp(B_cutoff, PUBLIC).negate();
	IntFp tmp1 = IntFp(B_cutoff, PUBLIC) + zk_inp.negate();

	// Select the right candidate
	IntFp tmp = zk_choice_bit * (tmp1 + tmp0.negate()) + tmp0;

	// finish the cutoff check
	{
		uint64_t cur = 1;
		for(int i = 0; i < needed_bits_special_range_check; i++) {
			tmp = tmp + zk_special_range_check_bits[i] * IntFp(cur, PUBLIC).negate();
			cur <<= 1;
		}
	}

	zk_zero_checking.emplace_back(tmp);

	// update the effective sum
	effective_sum += inp * choice_bit;
	zk_effective_sum = zk_effective_sum + zk_inp * zk_choice_bit;

	// update the effective count
	effective_count += choice_bit;
	zk_effective_count = zk_effective_count + zk_choice_bit;
}

void finalize_trimmed_mean(uint64_t effective_sum, IntFp zk_effective_sum, int reference_n, uint64_t effective_count, IntFp zk_effective_count, uint64_t &trimmed_mean, IntFp &zk_trimmed_mean, int fixed_point_shift, vector<IntFp> &zk_zero_checking) {
	// Store mean in fixed-point
	trimmed_mean = effective_sum * fixed_point_shift / effective_count;
	zk_trimmed_mean = IntFp(trimmed_mean, ALICE);

	// Check that: #subsamples * mean_shifted <= sum * fixed_point_shift < #subsamples * mean_shifted_plus_one

	uint64_t gap = effective_sum * fixed_point_shift - trimmed_mean * effective_count;
	IntFp zk_gap = zk_effective_sum * fixed_point_shift + zk_trimmed_mean * zk_effective_count.negate();

	// gap should be >= 0
	// gap should be < #samples
	//
	// ideally, it should be < #subsamples, but we don't have a straightforward way to test so
	range_check(gap, zk_gap, 0, reference_n * fixed_point_shift - 1, zk_zero_checking);

	// gap should be < #subsamples
	//
	// This is converted into #subsamples - gap should be greater than 0
	uint64_t gap_2 = effective_count - gap;
	IntFp zk_gap_2 = zk_effective_count + zk_gap.negate();
	range_check(gap_2, zk_gap_2, 1, reference_n, zk_zero_checking);
}

#endif //EMP_ZK_TRIMMED_MEAN_H
