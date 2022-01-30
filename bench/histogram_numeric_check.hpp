#ifndef EMP_ZK_HISTOGRAM_NUMERIC_CHECK_H
#define EMP_ZK_HISTOGRAM_NUMERIC_CHECK_H

#include "utils.hpp"

typedef struct _histogram_numeric_info {
	int needed_bits_group_check;
	int num_group;
	vector<uint64_t> group_start;
	vector<uint64_t> group_end;
} histogram_numeric_info;

histogram_numeric_info histogram_numeric_init(vector<uint64_t> group_start, vector<uint64_t> group_end) {
	assert(group_start.size() == group_end.size());

	histogram_numeric_info res;

	int num_group = group_start.size();

	int max_group_size = 0;
	for(int i = 0; i < num_group; i++) {
		int group_size = group_end[i] - group_start[i];
		if(group_size > max_group_size) {
			max_group_size = group_size;
		}
	}
	int needed_bits_group_check = ceil(log2(max_group_size + 1));

	res.needed_bits_group_check = needed_bits_group_check;
	res.num_group = num_group;
	res.group_start = group_start;
	res.group_end = group_end;

	return res;
}

void histogram_numeric_check(uint64_t inp, IntFp zk_inp, const histogram_numeric_info &info, uint64_t &res_group, IntFp &zk_res_group, vector<uint64_t> &group_counts, vector<IntFp> &zk_group_counts, vector<IntFp> &zk_zero_checking) {
	vector<bool> inp_group;

	// find out which group, the delta to the start, and the delta to the end
	uint64_t inp_group_delta_start = 0;
	uint64_t inp_group_delta_end = 0;

	for(int i = 0; i < info.num_group; i++) {
		if(inp >= info.group_start[i] && inp <= info.group_end[i]) {
			inp_group.push_back(true);
			inp_group_delta_start = inp - info.group_start[i];
			inp_group_delta_end = info.group_end[i] - inp;
		} else {
			inp_group.push_back(false);
		}
	}

	// allocate the group bits
	IntFp* zk_inp_group = new IntFp[info.num_group];
	for (int i = 0; i < info.num_group; i++) {
		zk_inp_group[i] = IntFp(inp_group[i], ALICE);
	}

	// perform bit checking of the group bits
	for (int i = 0; i < info.num_group; i++) {
		IntFp tmp;
		tmp = zk_inp_group[i] * zk_inp_group[i];
		tmp = tmp + zk_inp_group[i].negate();

		zk_zero_checking.emplace_back(tmp);
	}

	// perform sum checking of the group bits
	{
		IntFp tmp = IntFp(1, PUBLIC).negate();
		for (int i = 0; i < info.num_group; i++) {
			tmp = tmp + zk_inp_group[i];
		}
		zk_zero_checking.emplace_back(tmp);
	}

	// allocate the delta bits
	bool* delta_bits = new bool[info.needed_bits_group_check * 2];

	for (int j = 0; j < info.needed_bits_group_check; j++) {
		delta_bits[j] = inp_group_delta_start & 1;
		inp_group_delta_start >>= 1;

		delta_bits[info.needed_bits_group_check + j] = inp_group_delta_end & 1;
		inp_group_delta_end >>= 1;
	}

	IntFp* zk_delta_bits = new IntFp[info.needed_bits_group_check * 2];
	for (int i = 0; i < info.needed_bits_group_check * 2; i++) {
		zk_delta_bits[i] = IntFp(delta_bits[i], ALICE);
	}

	// perform bit checking of the delta bits
	for (int i = 0; i < info.needed_bits_group_check * 2; i++) {
		IntFp tmp;
		tmp = zk_delta_bits[i] * zk_delta_bits[i];
		tmp = tmp + zk_delta_bits[i].negate();

		zk_zero_checking.emplace_back(tmp);
	}

	// Check that each value is of the correct range
	// y[i] = (inp[i] - B_???) - \sum_{0..} (bits[j] * 2^j)
	{
		IntFp tmp0, tmp1;
		tmp0 = zk_inp.negate();
		tmp1 = zk_inp;

		for (int i = 0; i < info.num_group; i++) {
			tmp0 = tmp0 + IntFp(info.group_start[i], PUBLIC) * zk_inp_group[i];
			tmp1 = tmp1 + IntFp(info.group_end[i], PUBLIC) * zk_inp_group[i].negate();
		}

		uint64_t cur = 1;
		for (int i = 0; i < info.needed_bits_group_check; ++i) {
			tmp0 = tmp0 + zk_delta_bits[i] * cur;
			tmp1 = tmp1 + zk_delta_bits[info.needed_bits_group_check + i] * cur;

			cur <<= 1;
		}

		zk_zero_checking.emplace_back(tmp0);
		zk_zero_checking.emplace_back(tmp1);
	}

	// update the counts
	for(int i = 0; i < info.num_group; i++) {
		group_counts[i] = group_counts[i] + inp_group[i];
		zk_group_counts[i] = zk_group_counts[i] + zk_inp_group[i];
	}

	// save the group id if needed
	res_group = 0;
	zk_res_group = IntFp(0, PUBLIC);
	for (int i = 0; i < info.num_group; i++) {
		res_group = res_group + i * inp_group[i];
		zk_res_group = zk_res_group + IntFp(i, PUBLIC) * zk_inp_group[i];
	}

	delete [] zk_inp_group;
	delete [] delta_bits;
	delete [] zk_delta_bits;
}

#endif //EMP_ZK_HISTOGRAM_NUMERIC_CHECK_H
