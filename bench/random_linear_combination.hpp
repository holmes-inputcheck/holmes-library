#ifndef EMP_ZK_RANDOM_LINEAR_COMBINATION_H
#define EMP_ZK_RANDOM_LINEAR_COMBINATION_H

#include "utils.hpp"

void random_linear_combination(vector<uint64_t> inp, vector<IntFp> zk_inp, uint64_t challenge, uint64_t &res, IntFp &zk_res){
	assert(inp.size() == zk_inp.size());

	res = 0;
	zk_res = IntFp(0, PUBLIC);
	int len = inp.size();

	uint64_t cur = 1;
	for (int i = 0; i < len; i++) {
		res = add_mod(res, mult_mod(cur, inp[i]));
		zk_res = zk_res + zk_inp[i] * cur;

		cur = mult_mod(cur, challenge);
	}
}

#endif //EMP_ZK_RANDOM_LINEAR_COMBINATION_H
