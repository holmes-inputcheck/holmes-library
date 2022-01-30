#include "omp.h"

#include "host_ip.hpp"

#include "bench_dataset_1.hpp"
#include "range_check.hpp"
#include "histogram_numeric_check.hpp"
#include "jl_projector.hpp"
#include "mean_check.hpp"
#include "variance_check.hpp"
#include "random_linear_combination.hpp"

int port, party;
const int threads = 32;

int main(int argc, char** argv) {
	parse_party_and_port(argv, &party, &port);
	BoolIO<NetIO>* ios[threads+1];
	for(int i = 0; i < threads+1; ++i)
		ios[i] = new BoolIO<NetIO>(new NetIO(party == ALICE?nullptr:BENCH_HOST_IP,port+i), party==ALICE);

	std::cout << std::endl << "------------ dataset 1 ------------" << std::endl << std::endl;

	auto total_time_start = clock_start();

	setup_zk_bool<BoolIO<NetIO>>(ios, threads, party);
	setup_zk_arith<BoolIO<NetIO>>(ios, threads, party);
	
	vector<IntFp> zk_zero_checking;
	vector<uint64_t> precomputed_dataset;
	vector<IntFp> zk_precomputed_dataset;

	/************************************************************************************/

	int num_of_records = 411880;

	if(party == ALICE) {
		cout << "start to load the dataset, used OT triples = " << ZKFpExec::zk_exec->print_total_triple() << endl;
	}

	auto dataset = load_dataset(precomputed_dataset, zk_precomputed_dataset, num_of_records);

	if(party == ALICE) {
		cout << "num of entries: " << dataset.size() << endl;
		cout << "finished loading the dataset, used OT triples = " << ZKFpExec::zk_exec->print_total_triple() << endl;
	}

	vector<uint64_t> group_start {
			17, 20, 30, 40, 50, 60, 70, 80, 90
	};
	vector<uint64_t> group_end {
			19, 29, 39, 49, 59, 69, 79, 89, 98
	};

	auto age_group_info = histogram_numeric_init(group_start, group_end);

	int k = 40;
	vector<uint64_t> limit = {9, 12, 4, 8};
	auto jl_info = jl_projector_init(k, limit);

	vector<uint64_t> group_counts;
	vector<IntFp> zk_group_counts;
	for(int i = 0; i < age_group_info.num_group; i++) {
		group_counts.push_back(0);
		zk_group_counts.push_back(IntFp(0, PUBLIC));
	}

	vector<uint64_t> jl_res_projected;
	vector<IntFp> zk_jl_res_projected;
	for(int i = 0; i < k; i++) {
		jl_res_projected.push_back(0);
		zk_jl_res_projected.push_back(IntFp(0, PUBLIC));
	}

	for(int i = 0; i < dataset.size(); i++) {
		range_check(dataset[i].inp_job, dataset[i].zk_job, 1, 12, zk_zero_checking);
		range_check(dataset[i].inp_marital, dataset[i].zk_marital, 1, 4, zk_zero_checking);
		range_check(dataset[i].inp_education, dataset[i].zk_education, 1, 8, zk_zero_checking);
		range_check(dataset[i].inp_default, dataset[i].zk_default, 0, 2, zk_zero_checking);
		range_check(dataset[i].inp_housing, dataset[i].zk_housing, 0, 2, zk_zero_checking);
		range_check(dataset[i].inp_loan, dataset[i].zk_loan, 0, 2, zk_zero_checking);
		range_check(dataset[i].inp_contact, dataset[i].zk_contact, 1, 2, zk_zero_checking);
		range_check(dataset[i].inp_month, dataset[i].zk_month, 3, 12, zk_zero_checking);
		range_check(dataset[i].inp_day_of_week, dataset[i].zk_day_of_week, 1, 5, zk_zero_checking);
		range_check(dataset[i].inp_duration, dataset[i].zk_duration, 0, 4918, zk_zero_checking);
		range_check(dataset[i].inp_campaign, dataset[i].zk_campaign, 1, 56, zk_zero_checking);
		range_check(dataset[i].inp_pdays, dataset[i].zk_pdays, 0, 999, zk_zero_checking);
		range_check(dataset[i].inp_previous, dataset[i].zk_previous, 0, 7, zk_zero_checking);
		range_check(dataset[i].inp_poutcome, dataset[i].zk_poutcome, 0, 2, zk_zero_checking);
		range_check(dataset[i].inp_emp_var_rate, dataset[i].zk_emp_var_rate, 0, 48, zk_zero_checking);
		range_check(dataset[i].inp_cons_price_idx, dataset[i].zk_cons_price_idx, 92201, 94767, zk_zero_checking);
		range_check(dataset[i].inp_cons_conf_idx, dataset[i].zk_cons_conf_idx, 0, 239, zk_zero_checking);
		range_check(dataset[i].inp_euribor3m, dataset[i].zk_euribor3m, 634, 5045, zk_zero_checking);
		range_check(dataset[i].inp_nr_employed, dataset[i].zk_nr_employed, 49636, 52281, zk_zero_checking);
		range_check(dataset[i].inp_y, dataset[i].zk_y, 0, 1, zk_zero_checking);
	}

	if(party == ALICE) {
		cout << "finished all the range checks, used OT triples = " << ZKFpExec::zk_exec->print_total_triple() << endl;
	}

	for(int i = 0; i < dataset.size(); i++) {
		histogram_numeric_check(dataset[i].inp_age, dataset[i].zk_age, age_group_info, dataset[i].derived_age_group, dataset[i].zk_derived_age_group, group_counts, zk_group_counts, zk_zero_checking);
	}

	if(party == ALICE) {
		cout << "finished the counting, used OT triples = " << ZKFpExec::zk_exec->print_total_triple() << endl;
	}

	bool* prepared_bits[k];
	uint64_t* prepared_witnesses[k];

	for(int i = 0; i < k; i++) {
		prepared_bits[i] = new bool[dataset.size()];
		prepared_witnesses[i] = new uint64_t[dataset.size()];
	}

	#pragma omp parallel for default(shared)
	for(int i = 0; i < dataset.size(); i++) {
		jl_projector_prepare(dataset[i].derived_age_group, dataset[i].inp_job, dataset[i].inp_marital, dataset[i].inp_education,
					 jl_info, i, prepared_bits, prepared_witnesses);
	}

	for(int i = 0; i < dataset.size(); i++) {
		jl_projector(dataset[i].zk_derived_age_group, dataset[i].zk_job, dataset[i].zk_marital, dataset[i].zk_education,
							 jl_info, i, prepared_bits, prepared_witnesses, jl_res_projected, zk_jl_res_projected, zk_zero_checking);
	}

	for(int i = 0; i < k; i++) {
		delete[] prepared_bits[i];
		delete[] prepared_witnesses[i];
	}

	if(party == ALICE) {
		cout << "finished the JL, used OT triples = " << ZKFpExec::zk_exec->print_total_triple() << endl;
	}

	uint64_t duration_sum = 0;
	IntFp zk_duration_sum = IntFp(0, PUBLIC);

	for(int i = 0; i < dataset.size(); i++) {
		duration_sum += dataset[i].inp_duration;
		zk_duration_sum = zk_duration_sum + dataset[i].zk_duration;
	}

	uint64_t duration_mean = 0;
	IntFp zk_duration_mean = IntFp(0, PUBLIC);

	mean_check(duration_sum, zk_duration_sum, dataset.size(), duration_mean, zk_duration_mean, 100, zk_zero_checking);

	if(party == ALICE) {
		cout << "finished the mean checking, used OT triples = " << ZKFpExec::zk_exec->print_total_triple() << endl;
	}

	uint64_t duration_squared_sum = 0;
	IntFp zk_duration_squared_sum = IntFp(0, PUBLIC);

	for(int i = 0; i < dataset.size(); i++) {
		duration_squared_sum += dataset[i].inp_duration * dataset[i].inp_duration;
		zk_duration_squared_sum = zk_duration_squared_sum + dataset[i].zk_duration * dataset[i].zk_duration;
	}

	uint64_t duration_variance = 0;
	IntFp zk_duration_variance = IntFp(0, PUBLIC);

	// this one has a fixed point shift of 100
	variance_check(duration_sum, zk_duration_sum, duration_squared_sum, zk_duration_squared_sum, dataset.size(), duration_variance, zk_duration_variance, 100, zk_zero_checking);

	if(party == ALICE) {
		cout << "finished the variance checking, used OT triples = " << ZKFpExec::zk_exec->print_total_triple() << endl;
	}

	// Export precomputation

	// grouping results for age
	for(int i = 0; i < age_group_info.num_group; i++) {
		precomputed_dataset.push_back(group_counts[i]);
		zk_precomputed_dataset.push_back(zk_group_counts[i]);
	}

	// jl result
	for(int i = 0; i < k; i++) {
		precomputed_dataset.push_back(jl_res_projected[i]);
		zk_precomputed_dataset.push_back(zk_jl_res_projected[i]);
	}

	// mean and variances
	precomputed_dataset.push_back(duration_mean);
	zk_precomputed_dataset.push_back(zk_duration_mean);

	precomputed_dataset.push_back(duration_variance);
	zk_precomputed_dataset.push_back(zk_duration_variance);

	// our data = all the user inputs (41188 * 21)
	// 			+ counting (9)
	// 			+ jl result vector (6)
	// 			+ mean (duration, 1)
	// 			+ variance (duration, 1)

	// compute the random linear combination
	// temporarily, we set the challenges to be 7 and 8

	uint64_t randlc_res_1;
	IntFp zk_randlc_res_1 = IntFp(0, PUBLIC);

	uint64_t randlc_res_2;
	IntFp zk_randlc_res_2 = IntFp(0, PUBLIC);

	if(party == ALICE) {
		cout << "num of elements in the precomputed data = " << precomputed_dataset.size() << endl;
	}

	random_linear_combination(precomputed_dataset, zk_precomputed_dataset, 7, randlc_res_1, zk_randlc_res_1);
	random_linear_combination(precomputed_dataset, zk_precomputed_dataset, 8, randlc_res_2, zk_randlc_res_2);

	if(party == ALICE) {
		cout << "checksum 1 = " << randlc_res_1 << endl;
		cout << "checksum 2 = " << randlc_res_2 << endl;
	}

	// what actually happens in the consistency checking
	// 1. all parties agree on a random challenge (7)
	// 2. they will compute the result of the random linear combination over the precomputed data
	// 3. ask ZK to prove that the data in ZK will produce the same result
	//     - they tell ZK the challenge and the result, and ask the ZK to show that result-being-computed-in-ZK = result-provided.
	//     - this will introduce one zero-testing, which will batched together with others
	// 4. ask MPC to compute the result and reveal it (?) or prove that it is the same as the one computed outside

	/************************************************************************************/

	batch_reveal_check_zero(zk_zero_checking.data(), zk_zero_checking.size());
	jl_projector_clean(jl_info);

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
