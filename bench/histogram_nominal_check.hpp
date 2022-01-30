#ifndef EMP_ZK_HISTOGRAM_NOMINAL_CHECK_HPP
#define EMP_ZK_HISTOGRAM_NOMINAL_CHECK_HPP

#include "utils.hpp"

// Similar to the group check, except that now the number
// is already nominal.

void histogram_nominal_check(uint64_t inp, IntFp zk_inp, uint64_t start, uint64_t end, vector<uint64_t> &counts, vector<IntFp> &zk_counts, vector<IntFp> &zk_zero_checking) {
	assert(inp >= start);
	assert(inp <= end);
	
	vector<bool> inp_group;

	// compute the counting bits
	for(int i = start; i <= end; i++) {
		inp_group.push_back(inp == i);
	}

	// allocate the counting bits
	IntFp* zk_inp_group = new IntFp[end - start + 1];
	for (int i = 0; i < end - start + 1; i++) {
		zk_inp_group[i] = IntFp(inp_group[i], ALICE);
	}

	// perform bit checking of the counting bits
	for (int i = 0; i < end - start + 1; i++) {
		IntFp tmp;
		tmp = zk_inp_group[i] * zk_inp_group[i];
		tmp = tmp + zk_inp_group[i].negate();

		zk_zero_checking.emplace_back(tmp);
	}

	// perform sum checking of the counting bits
	{
		IntFp tmp = IntFp(1, PUBLIC).negate();
		for (int i = 0; i < end - start + 1; i++) {
			tmp = tmp + zk_inp_group[i];
		}
		zk_zero_checking.emplace_back(tmp);
	}

	// Check that each value matches the counting bits
	{
		IntFp tmp0;
		tmp0 = zk_inp.negate();

		for (int i = 0; i < end - start + 1; i++) {
			tmp0 = tmp0 + IntFp(start + i, PUBLIC) * zk_inp_group[i];
		}

		zk_zero_checking.emplace_back(tmp0);
	}

	// update the counts
	for(int i = 0; i < end - start + 1; i++) {
		counts[i] = counts[i] + inp_group[i];
		zk_counts[i] = zk_counts[i] + zk_inp_group[i];
	}

	delete [] zk_inp_group;
}

#endif //EMP_ZK_HISTOGRAM_NOMINAL_CHECK_HPP
