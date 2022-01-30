#include "utils.hpp"
#include "jl_projector.hpp"
#include "histogram_numeric_check.hpp"

bool is_test_passed(vector<IntFp> &zk_zero_checking) {
	for (int i = 0; i < zk_zero_checking.size(); i++) {
		if (zk_zero_checking.at(i).reveal() != 0) 
			return false;
	}
	return true;
}

/************************************************************************************/

struct range_check_args {
	uint64_t inp;
	IntFp zk_inp;
	uint64_t B_low;
	uint64_t B_high;
	vector<IntFp> *zk_zero_checking;
};

void range_check(uint64_t inp, IntFp zk_inp, uint64_t B_low, uint64_t B_high, vector<IntFp> &zk_zero_checking) {
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

	int delta_low = inp - B_low;
	int delta_high = B_high - inp;

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

void* test_range_check(void* arg) {
	struct range_check_args *range_check_args = (struct range_check_args*) arg;
	range_check_args->zk_zero_checking->clear();

	range_check(range_check_args->inp, range_check_args->zk_inp, range_check_args->B_low, range_check_args->B_high, *range_check_args->zk_zero_checking);
	pthread_exit(NULL);
	return NULL;
}

void run_range_check(bool valid_test, int &test_idx, int &range_test_passed, int &total_test_passed, uint64_t inp, IntFp zk_inp, uint64_t B_low, uint64_t B_high, vector<IntFp> &zk_zero_checking) {
	pthread_t test_thread;
	test_idx++;

	struct range_check_args range_check_args;
	range_check_args.inp = inp;
	range_check_args.zk_inp = zk_inp;
	range_check_args.B_low = B_low;
	range_check_args.B_high = B_high;
	range_check_args.zk_zero_checking = &zk_zero_checking;
	pthread_create(&test_thread, NULL, &test_range_check, (void *)&range_check_args);
	pthread_join(test_thread, NULL);

	if (valid_test && is_test_passed(zk_zero_checking)) {
		cout << "\033[0;32m[TEST " << test_idx << " PASSED]\033[0m Valid range check test passed!" << endl;
		range_test_passed++;
		total_test_passed++;
	} else if (valid_test && !is_test_passed(zk_zero_checking)) {
		cout << "\033[0;31m[TEST " << test_idx << " FAILED]\033[0m Valid range check test failed." << endl;
	} else if (!valid_test && !is_test_passed(zk_zero_checking)) {
		cout << "\033[0;32m[TEST " << test_idx << " PASSED]\033[0m Invalid range check test passed!" << endl;
		range_test_passed++;
		total_test_passed++;
	} else if (!valid_test && is_test_passed(zk_zero_checking)) {
		cout << "\033[0;31m[TEST " << test_idx << " FAILED]\033[0m Invalid range check test failed." << endl;
	}
}

/************************************************************************************/

struct mean_check_args {
	uint64_t inp_sum;
	int n;
	uint64_t mean;
	int fixed_point_shift;
	vector<IntFp> *zk_zero_checking;
};

void mean_check(uint64_t inp_sum, int n, uint64_t mean, int fixed_point_shift, vector<IntFp> &zk_zero_checking) {
	IntFp zk_inp_sum = IntFp(inp_sum, ALICE);
    IntFp zk_mean = IntFp(mean, ALICE);

	// Check that: #samples * (mean * fixed_point_shift) <= sum * fixed_point_shift < #samples * (mean * fixed_point_shift + 1)
	uint64_t gap = inp_sum * fixed_point_shift - mean * n;
	IntFp zk_gap = zk_inp_sum * fixed_point_shift + zk_mean.negate() * n;

	// gap should be >= 0
	// gap should be < #samples
	range_check(gap, zk_gap, 0, n - 1, zk_zero_checking);
}

void* test_mean_check(void* arg) {
	struct mean_check_args *mean_check_args = (struct mean_check_args*) arg;
	mean_check_args->zk_zero_checking->clear();

	mean_check(mean_check_args->inp_sum, mean_check_args->n, mean_check_args->mean, mean_check_args->fixed_point_shift, *mean_check_args->zk_zero_checking);
	pthread_exit(NULL);
	return NULL;
}

void run_mean_check(bool valid_test, int &test_idx, int &mean_test_passed, int &total_test_passed, uint64_t inp_sum, int n, uint64_t mean, int fixed_point_shift, vector<IntFp> &zk_zero_checking) {
	pthread_t test_thread;
	test_idx++;

	struct mean_check_args mean_check_args;
	mean_check_args.inp_sum = inp_sum;
	mean_check_args.n = n;
	mean_check_args.mean = mean;
	mean_check_args.fixed_point_shift = fixed_point_shift;
	mean_check_args.zk_zero_checking = &zk_zero_checking;
	pthread_create(&test_thread, NULL, &test_mean_check, (void *)&mean_check_args);
	pthread_join(test_thread, NULL);

	if (valid_test && is_test_passed(zk_zero_checking)) {
		cout << "\033[0;32m[TEST " << test_idx << " PASSED]\033[0m Valid mean check test passed!" << endl;
		mean_test_passed++;
		total_test_passed++;
	} else if (valid_test && !is_test_passed(zk_zero_checking)) {
		cout << "\033[0;31m[TEST " << test_idx << " FAILED]\033[0m Valid mean check test failed." << endl;
	} else if (!valid_test && !is_test_passed(zk_zero_checking)) {
		cout << "\033[0;32m[TEST " << test_idx << " PASSED]\033[0m Invalid mean check test passed!" << endl;
		mean_test_passed++;
		total_test_passed++;
	} else if (!valid_test && is_test_passed(zk_zero_checking)) {
		cout << "\033[0;31m[TEST " << test_idx << " FAILED]\033[0m Invalid mean check test failed." << endl;
	}
}

/************************************************************************************/

struct variance_check_args {
	uint64_t inp_sum;
	uint64_t inp_squared_sum;
	int n;
	uint64_t variance;
	int fixed_point_shift;
	vector<IntFp> *zk_zero_checking;
};

void variance_check(uint64_t inp_sum, uint64_t inp_squared_sum, int n, uint64_t variance, int fixed_point_shift, vector<IntFp> &zk_zero_checking) {
    IntFp zk_sum = IntFp(inp_sum, ALICE);
    IntFp zk_inp_squared_sum = IntFp(inp_squared_sum, ALICE);
	IntFp zk_variance = IntFp(variance, ALICE);

	uint64_t gap = inp_squared_sum * n * fixed_point_shift - inp_sum * inp_sum * fixed_point_shift - variance * n * n;
	IntFp zk_gap = zk_inp_squared_sum * n * fixed_point_shift + zk_sum.negate() * zk_sum * fixed_point_shift + zk_variance.negate() * n * n;

	// gap should be >= 0
	// gap should be < #samples * #samples
	range_check(gap, zk_gap, 0, (uint64_t)n * n - 1, zk_zero_checking);
}

void* test_variance_check(void* arg) {
	struct variance_check_args *var_check_args = (struct variance_check_args*) arg;
	var_check_args->zk_zero_checking->clear();

	variance_check(var_check_args->inp_sum, var_check_args->inp_squared_sum, var_check_args->n, var_check_args->variance, var_check_args->fixed_point_shift, *var_check_args->zk_zero_checking);
	pthread_exit(NULL);
	return NULL;
}

void run_variance_check(bool valid_test, int &test_idx, int &variance_test_passed, int &total_test_passed, uint64_t inp_sum, uint64_t inp_squared_sum, int n, uint64_t variance, int fixed_point_shift, vector<IntFp> &zk_zero_checking) {
	pthread_t test_thread;
	test_idx++;

	struct variance_check_args variance_check_args;
	variance_check_args.inp_sum = inp_sum;
	variance_check_args.inp_squared_sum = inp_squared_sum;
	variance_check_args.n = n;
	variance_check_args.variance = variance;
	variance_check_args.fixed_point_shift = fixed_point_shift;
	variance_check_args.zk_zero_checking = &zk_zero_checking;
	pthread_create(&test_thread, NULL, &test_variance_check, (void *)&variance_check_args);
	pthread_join(test_thread, NULL);

	if (valid_test && is_test_passed(zk_zero_checking)) {
		cout << "\033[0;32m[TEST " << test_idx << " PASSED]\033[0m Valid variance check test passed!" << endl;
		variance_test_passed++;
		total_test_passed++;
	} else if (valid_test && !is_test_passed(zk_zero_checking)) {
		cout << "\033[0;31m[TEST " << test_idx << " FAILED]\033[0m Valid variance check test failed." << endl;
	} else if (!valid_test && !is_test_passed(zk_zero_checking)) {
		cout << "\033[0;32m[TEST " << test_idx << " PASSED]\033[0m Invalid variance check test passed!" << endl;
		variance_test_passed++;
		total_test_passed++;
	} else if (!valid_test && is_test_passed(zk_zero_checking)) {
		cout << "\033[0;31m[TEST " << test_idx << " FAILED]\033[0m Invalid variance check test failed." << endl;
	}
}

/************************************************************************************/

struct trimmed_mean_check_args {
	uint64_t effective_sum;
	uint64_t effective_count;
	int reference_n;
	uint64_t trimmed_mean;
	int fixed_point_shift;
	vector<IntFp> *zk_zero_checking;
};

void trimmed_sum(uint64_t inp, int B_low, int B_cutoff, int B_high, uint64_t &effective_sum, uint64_t &effective_count, vector<IntFp> &zk_zero_checking) {
	assert(inp >= B_low);
	assert(inp <= B_high);

	IntFp zk_inp = IntFp(inp, ALICE);

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

	// update the effective count
	effective_count += choice_bit;
}

void trimmed_mean_check(uint64_t effective_sum, uint64_t effective_count, int reference_n, uint64_t trimmed_mean, int fixed_point_shift, vector<IntFp> &zk_zero_checking) {
	// Store trimmed mean in fixed-point
	IntFp zk_trimmed_mean = IntFp(trimmed_mean, ALICE);
	IntFp zk_effective_sum = IntFp(effective_sum, ALICE);
	IntFp zk_effective_count = IntFp(effective_count, ALICE);

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

void* test_trimmed_mean_check(void* arg) {
	struct trimmed_mean_check_args *trimmed_mean_check_args = (struct trimmed_mean_check_args*) arg;
	trimmed_mean_check_args->zk_zero_checking->clear();

	trimmed_mean_check(trimmed_mean_check_args->effective_sum, trimmed_mean_check_args->effective_count, trimmed_mean_check_args->reference_n, trimmed_mean_check_args->trimmed_mean, trimmed_mean_check_args->fixed_point_shift, *trimmed_mean_check_args->zk_zero_checking);
	pthread_exit(NULL);
	return NULL;
}

void run_trimmed_mean_check(bool valid_test, int &test_idx, int &trimmed_mean_test_passed, int &total_test_passed, uint64_t effective_sum, uint64_t effective_count, int reference_n, uint64_t trimmed_mean, int fixed_point_shift, vector<IntFp> &zk_zero_checking) {
	pthread_t test_thread;
	test_idx++;

	struct trimmed_mean_check_args trimmed_mean_check_args;
	trimmed_mean_check_args.effective_sum = effective_sum;
	trimmed_mean_check_args.effective_count = effective_count;
	trimmed_mean_check_args.reference_n = reference_n;
	trimmed_mean_check_args.trimmed_mean = trimmed_mean;
	trimmed_mean_check_args.fixed_point_shift = fixed_point_shift;
	trimmed_mean_check_args.zk_zero_checking = &zk_zero_checking;
	pthread_create(&test_thread, NULL, &test_trimmed_mean_check, (void *)&trimmed_mean_check_args);
	pthread_join(test_thread, NULL);

	if (valid_test && is_test_passed(zk_zero_checking)) {
		cout << "\033[0;32m[TEST " << test_idx << " PASSED]\033[0m Valid trimmed mean check test passed!" << endl;
		trimmed_mean_test_passed++;
		total_test_passed++;
	} else if (valid_test && !is_test_passed(zk_zero_checking)) {
		cout << "\033[0;31m[TEST " << test_idx << " FAILED]\033[0m Valid trimmed mean check test failed." << endl;
	} else if (!valid_test && !is_test_passed(zk_zero_checking)) {
		cout << "\033[0;32m[TEST " << test_idx << " PASSED]\033[0m Invalid trimmed mean check test passed!" << endl;
		trimmed_mean_test_passed++;
		total_test_passed++;
	} else if (!valid_test && is_test_passed(zk_zero_checking)) {
		cout << "\033[0;31m[TEST " << test_idx << " FAILED]\033[0m Invalid trimmed mean check test failed." << endl;
	}
}

/************************************************************************************/

struct histogram_nom_check_args {
	int n;
	uint64_t B_low;
	uint64_t B_high;
	uint64_t* sample_data;
	vector<uint64_t> nominal_counts;
	vector<IntFp> *zk_zero_checking;
};

void histogram_nominal_check(uint64_t inp, uint64_t start, uint64_t end, vector<IntFp> &zk_counts, vector<IntFp> &zk_zero_checking) {
	assert(inp >= start);
	assert(inp <= end);
	IntFp zk_inp = IntFp(inp, ALICE);

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

		if (tmp.reveal() != 0) {
			cout << "one" << endl;
		}
		zk_zero_checking.emplace_back(tmp);
	}

	// perform sum checking of the counting bits
	{
		IntFp tmp = IntFp(1, PUBLIC).negate();
		for (int i = 0; i < end - start + 1; i++) {
			tmp = tmp + zk_inp_group[i];
		}

		if (tmp.reveal() != 0) {
			cout << "two" << endl;
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

		if (tmp0.reveal() != 0) {
			cout << "three" << endl;
		}
		zk_zero_checking.emplace_back(tmp0);
	}

	// update the counts
	for(int i = 0; i < end - start + 1; i++) {
		zk_counts[i] = zk_counts[i] + zk_inp_group[i];
	}

	delete [] zk_inp_group;
}

void* test_histogram_nom_check(void* arg) {
	struct histogram_nom_check_args *histogram_nom_check_args = (struct histogram_nom_check_args*) arg;
	histogram_nom_check_args->zk_zero_checking->clear();
	
	vector<IntFp> zk_nominal_counts;
	for (int i = 0; i < histogram_nom_check_args->B_high - histogram_nom_check_args->B_low + 1; i++) {
		zk_nominal_counts.push_back(IntFp(0, PUBLIC));
	}

	for(int i = 0; i < histogram_nom_check_args->n; i++) {
		histogram_nominal_check(histogram_nom_check_args->sample_data[i], histogram_nom_check_args->B_low, histogram_nom_check_args->B_high, zk_nominal_counts, *histogram_nom_check_args->zk_zero_checking);
	}

	for (int i = 0; i < histogram_nom_check_args->B_high - histogram_nom_check_args->B_low + 1; i++) {
		histogram_nom_check_args->zk_zero_checking->emplace_back(IntFp(histogram_nom_check_args->nominal_counts[i], PUBLIC) + zk_nominal_counts[i].negate());
	}
	
	pthread_exit(NULL);
	return NULL;
}

void run_histogram_nom_check(bool valid_test, int &test_idx, int &histogram_nom_test_passed, int &total_test_passed, int n, uint64_t B_low, uint64_t B_high, uint64_t* sample_data, vector<uint64_t> nominal_counts, vector<IntFp> &zk_zero_checking) {
	pthread_t test_thread;
	test_idx++;

    struct histogram_nom_check_args histogram_nom_check_args;
	histogram_nom_check_args.n = n;
	histogram_nom_check_args.B_low = B_low;
	histogram_nom_check_args.B_high = B_high;
	histogram_nom_check_args.sample_data = sample_data;
	histogram_nom_check_args.nominal_counts = nominal_counts;
	histogram_nom_check_args.zk_zero_checking = &zk_zero_checking;
	pthread_create(&test_thread, NULL, &test_histogram_nom_check, (void *)&histogram_nom_check_args);
	pthread_join(test_thread, NULL);

	if (valid_test && is_test_passed(zk_zero_checking)) {
		cout << "\033[0;32m[TEST " << test_idx << " PASSED]\033[0m Valid histogram nominal check test passed!" << endl;
		histogram_nom_test_passed++;
		total_test_passed++;
	} else if (valid_test && !is_test_passed(zk_zero_checking)) {
		cout << "\033[0;31m[TEST " << test_idx << " FAILED]\033[0m Valid histogram nominal check test failed." << endl;
	} else if (!valid_test && !is_test_passed(zk_zero_checking)) {
		cout << "\033[0;32m[TEST " << test_idx << " PASSED]\033[0m Invalid histogram nominal check test passed!" << endl;
		histogram_nom_test_passed++;
		total_test_passed++;
	} else if (!valid_test && is_test_passed(zk_zero_checking)) {
		cout << "\033[0;31m[TEST " << test_idx << " FAILED]\033[0m Invalid histogram nominal check test failed." << endl;
	}
}


/************************************************************************************/
struct histogram_num_check_args {
	int n;
	uint64_t* sample_data;
	histogram_numeric_info info;
	vector<uint64_t> group_counts;
	vector<IntFp> *zk_zero_checking;
};

void histogram_numeric_check(uint64_t inp, const histogram_numeric_info &info, vector<IntFp> &zk_group_counts, vector<IntFp> &zk_zero_checking) {
	IntFp zk_inp = IntFp(inp, ALICE);

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
		zk_group_counts[i] = zk_group_counts[i] + zk_inp_group[i];
	}

	delete [] zk_inp_group;
	delete [] delta_bits;
	delete [] zk_delta_bits;
}

void* test_histogram_num_check(void* arg) {
	struct histogram_num_check_args *histogram_num_check_args = (struct histogram_num_check_args*) arg;
	histogram_num_check_args->zk_zero_checking->clear();
	
	vector<IntFp> zk_group_counts;
	for (int i = 0; i < histogram_num_check_args->info.num_group; i++) {
		zk_group_counts.push_back(IntFp(0, PUBLIC));
	}

	for(int i = 0; i < histogram_num_check_args->n; i++) {
		histogram_numeric_check(histogram_num_check_args->sample_data[i], histogram_num_check_args->info, zk_group_counts, *histogram_num_check_args->zk_zero_checking);
	}

	for (int i = 0; i < histogram_num_check_args->info.num_group; i++) {
		histogram_num_check_args->zk_zero_checking->emplace_back(IntFp(histogram_num_check_args->group_counts[i], PUBLIC) + zk_group_counts[i].negate());
	}
	
	pthread_exit(NULL);
	return NULL;
}

void run_histogram_num_check(bool valid_test, int &test_idx, int &histogram_num_test_passed, int &total_test_passed, int n, uint64_t* sample_data, histogram_numeric_info info, vector<uint64_t> group_counts, vector<IntFp> &zk_zero_checking) {
	pthread_t test_thread;
	test_idx++;

	struct histogram_num_check_args histogram_num_check_args;
	histogram_num_check_args.n = n;
	histogram_num_check_args.sample_data = sample_data;
	histogram_num_check_args.info = info;
	histogram_num_check_args.group_counts = group_counts;
	histogram_num_check_args.zk_zero_checking = &zk_zero_checking;
	pthread_create(&test_thread, NULL, &test_histogram_num_check, (void *)&histogram_num_check_args);
	pthread_join(test_thread, NULL);

	if (valid_test && is_test_passed(zk_zero_checking)) {
		cout << "\033[0;32m[TEST " << test_idx << " PASSED]\033[0m Valid histogram numeric check test passed!" << endl;
		histogram_num_test_passed++;
		total_test_passed++;
	} else if (valid_test && !is_test_passed(zk_zero_checking)) {
		cout << "\033[0;31m[TEST " << test_idx << " FAILED]\033[0m Valid histogram numeric check test failed." << endl;
	} else if (!valid_test && !is_test_passed(zk_zero_checking)) {
		cout << "\033[0;32m[TEST " << test_idx << " PASSED]\033[0m Invalid histogram numeric check test passed!" << endl;
		histogram_num_test_passed++;
		total_test_passed++;
	} else if (!valid_test && is_test_passed(zk_zero_checking)) {
		cout << "\033[0;31m[TEST " << test_idx << " FAILED]\033[0m Invalid histogram numeric check test failed." << endl;
	}
}

/************************************************************************************/
struct jl_check_args {
	int k;
	int n;
	uint64_t* sample_data_1;
	uint64_t* sample_data_2;
	uint64_t* sample_data_3;
	uint64_t* sample_data_4;
	jl_info info;
	bool** prepared_bits;
	uint64_t** prepared_witnesses;
	vector<IntFp> *zk_zero_checking;
};

void jl_projector(IntFp zk_inp_1, IntFp zk_inp_2, IntFp zk_inp_3, IntFp zk_inp_4, jl_info &info, uint64_t idx, bool** prepared_bits, uint64_t** prepared_witnesses, vector<IntFp> &zk_zero_checking) {
	// compute j
	IntFp zk_j = IntFp(0, PUBLIC);
	zk_j = zk_j + zk_inp_1 * info.limit[1] * info.limit[2] * info.limit[3];
	zk_j = zk_j + zk_inp_2 * info.limit[2] * info.limit[3];
	zk_j = zk_j + zk_inp_3 * info.limit[3];
	zk_j = zk_j + zk_inp_4 * 1;

	vector<uint64_t> res;
	vector<IntFp> zk_res;
	for(int i = 0; i < info.k; i++) {
		res.push_back(0);
		zk_res.push_back(IntFp(0, PUBLIC));
	}

	for(int i = 0; i < info.k; i++) {
		jl_query_prf_with_prepared_data(i, zk_j, info, idx, prepared_bits, prepared_witnesses, res[i], zk_res[i], zk_zero_checking);
	}
}

void* test_jl_check(void* arg) {
	struct jl_check_args *jl_check_args = (struct jl_check_args*) arg;
	jl_check_args->zk_zero_checking->clear();

	for(int i = 0; i < jl_check_args->n; i++) {
		IntFp zk_inp_1 = IntFp(jl_check_args->sample_data_1[i], ALICE);
		IntFp zk_inp_2 = IntFp(jl_check_args->sample_data_2[i], ALICE);
		IntFp zk_inp_3 = IntFp(jl_check_args->sample_data_3[i], ALICE);
		IntFp zk_inp_4 = IntFp(jl_check_args->sample_data_4[i], ALICE);
		jl_projector(zk_inp_1, zk_inp_2, zk_inp_3, zk_inp_4,
			jl_check_args->info, i, jl_check_args->prepared_bits, jl_check_args->prepared_witnesses, *jl_check_args->zk_zero_checking);
	}

	pthread_exit(NULL);
	return NULL;
}

void run_jl_check(bool valid_test, int &test_idx, int &jl_test_passed, int &total_test_passed, int n, uint64_t* sample_data_1, uint64_t* sample_data_2, uint64_t* sample_data_3, uint64_t* sample_data_4, jl_info info, bool** prepared_bits, uint64_t** prepared_witnesses, vector<IntFp> &zk_zero_checking) {
	pthread_t test_thread;
	test_idx++;

	struct jl_check_args jl_check_args;
	jl_check_args.n = n;
	jl_check_args.sample_data_1 = sample_data_1;
	jl_check_args.sample_data_2 = sample_data_2;
	jl_check_args.sample_data_3 = sample_data_3;
	jl_check_args.sample_data_4 = sample_data_4;
	jl_check_args.info = info;
	jl_check_args.prepared_bits = prepared_bits;
	jl_check_args.prepared_witnesses = prepared_witnesses;
	jl_check_args.zk_zero_checking = &zk_zero_checking;
	pthread_create(&test_thread, NULL, &test_jl_check, (void *)&jl_check_args);
	pthread_join(test_thread, NULL);

	if (valid_test && is_test_passed(zk_zero_checking)) {
		cout << "\033[0;32m[TEST " << test_idx << " PASSED]\033[0m Valid JL check test passed!" << endl;
		jl_test_passed++;
		total_test_passed++;
	} else if (valid_test && !is_test_passed(zk_zero_checking)) {
		cout << "\033[0;31m[TEST " << test_idx << " FAILED]\033[0m Valid JL check test failed." << endl;
	} else if (!valid_test && !is_test_passed(zk_zero_checking)) {
		cout << "\033[0;32m[TEST " << test_idx << " PASSED]\033[0m Invalid JL check test passed!" << endl;
		jl_test_passed++;
		total_test_passed++;
	} else if (!valid_test && is_test_passed(zk_zero_checking)) {
		cout << "\033[0;31m[TEST " << test_idx << " FAILED]\033[0m Invalid JL check test failed." << endl;
	}
}

/************************************************************************************/