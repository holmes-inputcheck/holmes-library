#include "host_ip.hpp"

#include "test_check.hpp"
#include "pthread.h"

int port, party;
const int threads = 32;

int main(int argc, char** argv) {
	parse_party_and_port(argv, &party, &port);
	BoolIO<NetIO>* ios[threads+1];
	for(int i = 0; i < threads+1; ++i)
		ios[i] = new BoolIO<NetIO>(new NetIO(party == ALICE?nullptr:BENCH_HOST_IP,port+i), party==ALICE);

	std::cout << std::endl << "------------ running unit tests ------------" << std::endl << std::endl;

	auto total_time_start = clock_start();

	setup_zk_bool<BoolIO<NetIO>>(ios, threads, party);
	setup_zk_arith<BoolIO<NetIO>>(ios, threads, party);
	
	vector<IntFp> zk_zero_checking;
	vector<uint64_t> precomputed_dataset;
	vector<IntFp> zk_precomputed_dataset;
	

	int test_idx = 0;
	int total_test_passed = 0;
	int triple_counter = 0;

	int n = 100;
	uint64_t sample_data[n];
	for (int i = 0; i < n; i++) {
		sample_data[i] = i;
	}
	int fixed_point_shift = 100;

	/************************************************************************************/

	cout << "\033[0;32m[INFO]\033[0m Running range checks." << endl;

	int range_test_passed = 0;
	int total_range_test = 3;
	run_range_check(true, test_idx, range_test_passed, total_test_passed, 5, IntFp(5, PUBLIC), 1, 12, zk_zero_checking);
	run_range_check(false, test_idx, range_test_passed, total_test_passed, 5, IntFp(8, PUBLIC), 1, 12, zk_zero_checking);
	run_range_check(false, test_idx, range_test_passed, total_test_passed, 15, IntFp(15, PUBLIC), 1, 12, zk_zero_checking);

	if(party == ALICE) {
		cout << "\033[0;32m[INFO]\033[0m " << range_test_passed << "/" << total_range_test << " range checks passed." << endl;
		cout << "finished all the range checks, used OT triples = " << ZKFpExec::zk_exec->print_total_triple() - triple_counter << endl << endl;
		triple_counter = ZKFpExec::zk_exec->print_total_triple();
	}

	/************************************************************************************/

	cout << "\033[0;32m[INFO]\033[0m Running mean checks." << endl;
	
	int mean_test_passed = 0;
	int total_mean_test = 5;

	uint64_t real_inp_sum = 0;
	for (int i = 0; i < n; i++) {
		real_inp_sum += sample_data[i];
	}
	uint64_t real_mean = real_inp_sum * fixed_point_shift / n;

	run_mean_check(true, test_idx, mean_test_passed, total_test_passed, real_inp_sum, n, real_mean, fixed_point_shift, zk_zero_checking);
	run_mean_check(false, test_idx, mean_test_passed, total_test_passed, real_inp_sum, n, real_mean + 1, fixed_point_shift, zk_zero_checking);
	run_mean_check(false, test_idx, mean_test_passed, total_test_passed, real_inp_sum, n + 1, real_mean, fixed_point_shift, zk_zero_checking);
	run_mean_check(false, test_idx, mean_test_passed, total_test_passed, real_inp_sum + 1, n, real_mean, fixed_point_shift, zk_zero_checking);
	run_mean_check(false, test_idx, mean_test_passed, total_test_passed, real_inp_sum, n, real_mean, fixed_point_shift + 1, zk_zero_checking);

	if(party == ALICE) {
		cout << "\033[0;32m[INFO]\033[0m " << mean_test_passed << "/" << total_mean_test << " mean checks passed." << endl;
		cout << "finished all the mean checks, used OT triples = " << ZKFpExec::zk_exec->print_total_triple() - triple_counter << endl << endl;
		triple_counter = ZKFpExec::zk_exec->print_total_triple();
	}

	/************************************************************************************/
	
	cout << "\033[0;32m[INFO]\033[0m Running variance checks." << endl;

	int variance_test_passed = 0;
	int total_variance_test = 6;

	uint64_t real_inp_squared_sum = 0;
	for (int i = 0; i < n; i++) {
		real_inp_squared_sum += sample_data[i] * sample_data[i];
	}
	uint64_t real_variance = (real_inp_squared_sum * n - real_inp_sum * real_inp_sum) * fixed_point_shift / n / n;
	
	run_variance_check(true, test_idx, variance_test_passed, total_test_passed, real_inp_sum, real_inp_squared_sum, n, real_variance, fixed_point_shift, zk_zero_checking);
	run_variance_check(false, test_idx, variance_test_passed, total_test_passed, real_inp_sum + 1, real_inp_squared_sum, n, real_variance, fixed_point_shift, zk_zero_checking);
	run_variance_check(false, test_idx, variance_test_passed, total_test_passed, real_inp_sum, real_inp_squared_sum + 1, n, real_variance, fixed_point_shift, zk_zero_checking);
	run_variance_check(false, test_idx, variance_test_passed, total_test_passed, real_inp_sum, real_inp_squared_sum, n + 1, real_variance, fixed_point_shift, zk_zero_checking);
	run_variance_check(false, test_idx, variance_test_passed, total_test_passed, real_inp_sum, real_inp_squared_sum, n, real_variance + 1, fixed_point_shift, zk_zero_checking);
	run_variance_check(false, test_idx, variance_test_passed, total_test_passed, real_inp_sum, real_inp_squared_sum, n, real_variance, fixed_point_shift + 1, zk_zero_checking);

	if(party == ALICE) {
		cout << "\033[0;32m[INFO]\033[0m " << variance_test_passed << "/" << total_variance_test << " variance checks passed." << endl;
		cout << "finished all the variance checks, used OT triples = " << ZKFpExec::zk_exec->print_total_triple() - triple_counter << endl << endl;
		triple_counter = ZKFpExec::zk_exec->print_total_triple();
	}
	
	/************************************************************************************/

	cout << "\033[0;32m[INFO]\033[0m Running trimmed mean checks." << endl;

	int trimmed_mean_test_passed = 0;
	int total_trimmed_mean_test = 6;

	// Prepare valid data
	uint64_t real_effective_count = 0;
	uint64_t real_effective_sum = 0;
	for (int i = 0; i < n; i++) {
		trimmed_sum(sample_data[i], 0, 50, 99, real_effective_sum, real_effective_count, zk_zero_checking);
	}

	// Prepare invalid data
	uint64_t fake_effective_count = 0;
	uint64_t fake_effective_sum = 0;
	for (int i = 0; i < n; i++) {
		trimmed_sum(sample_data[i], 0, 51, 99, fake_effective_sum, fake_effective_count, zk_zero_checking);
	}

	uint64_t real_trimmed_mean = real_effective_sum * fixed_point_shift / real_effective_count;
	
	run_trimmed_mean_check(true, test_idx, trimmed_mean_test_passed, total_test_passed, real_effective_sum, real_effective_count, n, real_trimmed_mean, fixed_point_shift, zk_zero_checking);
	run_trimmed_mean_check(false, test_idx, trimmed_mean_test_passed, total_test_passed, fake_effective_sum, fake_effective_count, n, real_trimmed_mean, fixed_point_shift, zk_zero_checking);
	run_trimmed_mean_check(false, test_idx, trimmed_mean_test_passed, total_test_passed, real_effective_sum + 1, real_effective_count, n, real_trimmed_mean, fixed_point_shift, zk_zero_checking);
	run_trimmed_mean_check(false, test_idx, trimmed_mean_test_passed, total_test_passed, real_effective_sum, real_effective_count + 1, n, real_trimmed_mean, fixed_point_shift, zk_zero_checking);
	run_trimmed_mean_check(false, test_idx, trimmed_mean_test_passed, total_test_passed, real_effective_sum, real_effective_count, n, real_trimmed_mean + 1, fixed_point_shift, zk_zero_checking);
	run_trimmed_mean_check(false, test_idx, trimmed_mean_test_passed, total_test_passed, real_effective_sum, real_effective_count, n, real_trimmed_mean, fixed_point_shift + 1, zk_zero_checking);

	if(party == ALICE) {
		cout << "\033[0;32m[INFO]\033[0m " << trimmed_mean_test_passed << "/" << total_trimmed_mean_test << " trimmed mean checks passed." << endl;
		cout << "finished all the trimmed mean checks, used OT triples = " << ZKFpExec::zk_exec->print_total_triple() - triple_counter << endl << endl;
		triple_counter = ZKFpExec::zk_exec->print_total_triple();
	}

	/************************************************************************************/

	cout << "\033[0;32m[INFO]\033[0m Running histogram nominal checks." << endl;

	int histogram_nom_test_passed = 0;
	int total_histogram_nom_test = 4;

	vector<uint64_t> nominal_counts;
	for (int i = 0; i < n; i++) {
		nominal_counts.emplace_back(1);
	}

	vector<uint64_t> extended_nominal_counts;
	for (int i = 0; i < n; i++) {
		extended_nominal_counts.emplace_back(1);
	}
	for (int i = 0; i < 50; i++) {
		extended_nominal_counts.emplace_back(0);
	}

	uint64_t nom_sample_data_2[n];
	int step = 10;
	for (int i = 0; i < n; i += step) {
		for (int j = 0; j < step; j++) {
			nom_sample_data_2[i + j] = i;
		}
	}

	vector<uint64_t> nominal_counts_2;
	for (int i = 0; i < n; i ++) {
		if (i % 10 == 0) {
			nominal_counts_2.emplace_back(10);
		} else {
			nominal_counts_2.emplace_back(0);
		}
	}

	vector<uint64_t> partial_nominal_counts_2;
	for (int i = 0; i < 40; i++) {
		if (i % 10 == 0) {
			nominal_counts_2.emplace_back(10);
		} else {
			nominal_counts_2.emplace_back(0);
		}
	}

	run_histogram_nom_check(true, test_idx, histogram_nom_test_passed, total_test_passed, n, 0, 99, sample_data, nominal_counts, zk_zero_checking);
	run_histogram_nom_check(true, test_idx, histogram_nom_test_passed, total_test_passed, n, 0, 90, nom_sample_data_2, nominal_counts_2, zk_zero_checking);
	run_histogram_nom_check(true, test_idx, histogram_nom_test_passed, total_test_passed, n, 0, 149, sample_data, extended_nominal_counts, zk_zero_checking);
	run_histogram_nom_check(false, test_idx, histogram_nom_test_passed, total_test_passed, n - 1, 0, 99, sample_data, nominal_counts, zk_zero_checking);
		
	if(party == ALICE) {
		cout << "\033[0;32m[INFO]\033[0m " << histogram_nom_test_passed << "/" << total_histogram_nom_test << " histogram nominal checks passed." << endl;
		cout << "finished all the histogram nominal checks, used OT triples = " << ZKFpExec::zk_exec->print_total_triple() - triple_counter << endl << endl;
		triple_counter = ZKFpExec::zk_exec->print_total_triple();
	}

	/************************************************************************************/

	cout << "\033[0;32m[INFO]\033[0m Running histogram numeric checks." << endl;

	int histogram_num_test_passed = 0;
	int total_histogram_num_test = 4;

	vector<uint64_t> real_group_start {
			0, 10, 20, 30, 40, 50, 60, 70, 80, 90
	};

	vector<uint64_t> real_group_end {
			9, 19, 29, 39, 49, 59, 69, 79, 89, 99
	};

	vector<uint64_t> real_group_counts {
			10, 10, 10, 10, 10, 10, 10, 10, 10, 10
	};

	auto real_group_info = histogram_numeric_init(real_group_start, real_group_end);

	vector<uint64_t> fake_group_start {
			0, 10, 20, 30, 40, 50, 60, 70, 80
	};

	vector<uint64_t> fake_group_end {
			9, 19, 29, 39, 49, 59, 69, 79, 89
	};

	vector<uint64_t> fake_group_counts {
			10, 10, 10, 10, 10, 10, 10, 10, 10
	};

	auto fake_group_info = histogram_numeric_init(fake_group_start, fake_group_end);

	run_histogram_num_check(true, test_idx, histogram_num_test_passed, total_test_passed, n, sample_data, real_group_info, real_group_counts, zk_zero_checking);
	run_histogram_num_check(true, test_idx, histogram_num_test_passed, total_test_passed, n - 10, sample_data, fake_group_info, fake_group_counts, zk_zero_checking);

	sample_data[0] += 10;
	run_histogram_num_check(false, test_idx, histogram_num_test_passed, total_test_passed, n, sample_data, real_group_info, real_group_counts, zk_zero_checking);
	sample_data[0] -= 10;

	run_histogram_num_check(false, test_idx, histogram_num_test_passed, total_test_passed, n, sample_data, fake_group_info, fake_group_counts, zk_zero_checking);
	
	if(party == ALICE) {
		cout << "\033[0;32m[INFO]\033[0m " << histogram_num_test_passed << "/" << total_histogram_num_test << " histogram numeric checks passed." << endl;
		cout << "finished all the histogram numeric checks, used OT triples = " << ZKFpExec::zk_exec->print_total_triple() - triple_counter << endl << endl;
		triple_counter = ZKFpExec::zk_exec->print_total_triple();
	}

	/************************************************************************************/

	cout << "\033[0;32m[INFO]\033[0m Running JL checks." << endl;

	int jl_test_passed = 0;
	int total_jl_test = 8;

	uint64_t limit_1 = n - 1;

	uint64_t sample_data_2[n];
	for (int i = 0; i < n; i++) {
		sample_data_2[i] = i * i;
	}
	uint64_t limit_2 = (n - 1) * (n - 1) + 1;

	uint64_t sample_data_3[n];
	for (int i = 0; i < n; i++) {
		sample_data_3[i] = i * 2;
	}
	uint64_t limit_3 = 2 * (n - 1) + 1;

	uint64_t sample_data_4[n];
	for (int i = 0; i < n; i++) {
		sample_data_4[i] = i + 25;
	}
	uint64_t limit_4 = n + 25;

	int k = 4;

	vector<uint64_t> real_limit = {limit_1, limit_2, limit_3, limit_4};
	auto real_jl_info = jl_projector_init(k, real_limit);

	vector<uint64_t> fake_limit = {limit_1, limit_2 - 1, limit_3, limit_4};
	auto fake_jl_info = jl_projector_init(k, fake_limit);

	bool* real_prepared_bits[k];
	uint64_t* real_prepared_witnesses[k];

	bool* fake_prepared_bits[k];
	uint64_t* fake_prepared_witnesses[k];

	bool* fake_prepared_bits_2[k];
	uint64_t* fake_prepared_witnesses_2[k];


	for(int i = 0; i < k; i++) {
		real_prepared_bits[i] = new bool[n];
		real_prepared_witnesses[i] = new uint64_t[n];
		fake_prepared_bits[i] = new bool[n];
		fake_prepared_witnesses[i] = new uint64_t[n];
		fake_prepared_bits_2[i] = new bool[n];
		fake_prepared_witnesses_2[i] = new uint64_t[n];
	}

	// Prepare valid data
	#pragma omp parallel for default(shared)
	for(int i = 0; i < n; i++) {
		jl_projector_prepare(sample_data[i], sample_data_2[i], sample_data_3[i], sample_data_4[i],
					 real_jl_info, i, real_prepared_bits, real_prepared_witnesses);
	}

	// Prepare invalid data
	int jl_prep_idx = 0;
	jl_projector_prepare(sample_data[jl_prep_idx] + 1, sample_data_2[jl_prep_idx], sample_data_3[jl_prep_idx], sample_data_4[jl_prep_idx],
					 real_jl_info, jl_prep_idx, fake_prepared_bits, fake_prepared_witnesses);
	#pragma omp parallel for default(shared)
	for(int i = 1; i < n; i++) {
		jl_projector_prepare(sample_data[i], sample_data_2[i], sample_data_3[i], sample_data_4[i],
					 real_jl_info, i, fake_prepared_bits, fake_prepared_witnesses);
	}
	
	#pragma omp parallel for default(shared)
	for(int i = 0; i < n; i++) {
		jl_projector_prepare(sample_data[i], sample_data_2[i], sample_data_3[i], sample_data_4[i],
					 fake_jl_info, i, fake_prepared_bits_2, fake_prepared_witnesses_2);
	}

	run_jl_check(true, test_idx, jl_test_passed, total_test_passed, n, sample_data, sample_data_2, sample_data_3, sample_data_4, real_jl_info, real_prepared_bits, real_prepared_witnesses, zk_zero_checking);
	run_jl_check(false, test_idx, jl_test_passed, total_test_passed, n, sample_data, sample_data_2, sample_data_3, sample_data_4, real_jl_info, fake_prepared_bits, real_prepared_witnesses, zk_zero_checking);
	run_jl_check(false, test_idx, jl_test_passed, total_test_passed, n, sample_data, sample_data_2, sample_data_3, sample_data_4, real_jl_info, real_prepared_bits, fake_prepared_witnesses, zk_zero_checking);
	run_jl_check(false, test_idx, jl_test_passed, total_test_passed, n + 1, sample_data, sample_data_2, sample_data_3, sample_data_4, real_jl_info, real_prepared_bits, real_prepared_witnesses, zk_zero_checking);

	sample_data[0] += 1;
	run_jl_check(false, test_idx, jl_test_passed, total_test_passed, n, sample_data, sample_data_2, sample_data_3, sample_data_4, real_jl_info, real_prepared_bits, real_prepared_witnesses, zk_zero_checking);
	sample_data[0] -= 1;

	run_jl_check(false, test_idx, jl_test_passed, total_test_passed, n, sample_data, sample_data_2, sample_data_3, sample_data_4, fake_jl_info, real_prepared_bits, fake_prepared_witnesses_2, zk_zero_checking);
	run_jl_check(false, test_idx, jl_test_passed, total_test_passed, n, sample_data, sample_data_2, sample_data_3, sample_data_4, real_jl_info, fake_prepared_bits_2, real_prepared_witnesses, zk_zero_checking);
	run_jl_check(false, test_idx, jl_test_passed, total_test_passed, n, sample_data, sample_data_2, sample_data_3, sample_data_4, fake_jl_info, real_prepared_bits, real_prepared_witnesses, zk_zero_checking);

	for(int i = 0; i < k; i++) {
		delete[] real_prepared_bits[i];
		delete[] real_prepared_witnesses[i];
		delete[] fake_prepared_bits[i];
		delete[] fake_prepared_witnesses[i];
		delete[] fake_prepared_bits_2[i];
		delete[] fake_prepared_witnesses_2[i];
	}

	if(party == ALICE) {
		cout << "\033[0;32m[INFO]\033[0m " << jl_test_passed << "/" << total_jl_test << " JL checks passed." << endl;
		cout << "finished all the JL checks, used OT triples = " << ZKFpExec::zk_exec->print_total_triple() - triple_counter << endl << endl;
		triple_counter = ZKFpExec::zk_exec->print_total_triple();
	}

	/************************************************************************************/

	cout << "\033[0;32m[INFO]\033[0m " << total_test_passed << "/" << test_idx << " total unit tests passed." << endl;

	finalize_zk_bool<BoolIO<NetIO>>();
	finalize_zk_arith<BoolIO<NetIO>>();


	auto total_time = time_from(total_time_start);
	printf("total time: %f\n", total_time);
	{
		FILE * fp = fopen("./benchmark_result.txt", "w");
		fprintf(fp, "%f\n", total_time);
		fclose(fp);
	}

	for(int i = 0; i < threads+1; ++i) {
		delete ios[i]->io;
		delete ios[i];
	}
	return 0;
}
