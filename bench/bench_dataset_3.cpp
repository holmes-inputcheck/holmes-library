#include "host_ip.hpp"

#include "bench_dataset_3.hpp"
#include "range_check.hpp"
#include "trimmed_mean.hpp"
#include "random_linear_combination.hpp"

int port, party;
const int threads = 32;

int main(int argc, char** argv) {
	parse_party_and_port(argv, &party, &port);
	BoolIO<NetIO>* ios[threads+1];
	for(int i = 0; i < threads+1; ++i)
		ios[i] = new BoolIO<NetIO>(new NetIO(party == ALICE?nullptr:BENCH_HOST_IP,port+i), party==ALICE);

	std::cout << std::endl << "------------ dataset 3 ------------" << std::endl << std::endl;

	auto total_time_start = clock_start();

	setup_zk_bool<BoolIO<NetIO>>(ios, threads, party);
	setup_zk_arith<BoolIO<NetIO>>(ios, threads, party);

	vector<IntFp> zk_zero_checking;
	vector<uint64_t> precomputed_dataset;
	vector<IntFp> zk_precomputed_dataset;

	/************************************************************************************/

	int num_of_records = 567291;

	if(party == ALICE) {
		cout << "start to load the dataset, used OT triples = " << ZKFpExec::zk_exec->print_total_triple() << endl;
	}

	auto dataset = load_dataset(precomputed_dataset, zk_precomputed_dataset, num_of_records);

	if(party == ALICE) {
		cout << "num of entries: " << dataset.size() << endl;
		cout << "finished loading the dataset, used OT triples = " << ZKFpExec::zk_exec->print_total_triple() << endl;
	}

	for(int i = 0; i < dataset.size(); i++) {
		range_check(dataset[i].inp_date, dataset[i].zk_date, 1, 30, zk_zero_checking);
		range_check(dataset[i].inp_site_id, dataset[i].zk_site_id, 342, 351, zk_zero_checking);
		range_check(dataset[i].inp_ad_type_id, dataset[i].zk_ad_type_id, 10, 17, zk_zero_checking);
		range_check(dataset[i].inp_geo_id, dataset[i].zk_geo_id, 1, 308, zk_zero_checking);
		range_check(dataset[i].inp_device_category_id, dataset[i].zk_device_category_id, 1, 5, zk_zero_checking);
		range_check(dataset[i].inp_advertiser_id, dataset[i].zk_advertiser_id, 8, 2646, zk_zero_checking);
		range_check(dataset[i].inp_order_id, dataset[i].zk_order_id, 45, 3489, zk_zero_checking);
		range_check(dataset[i].inp_line_item_type_id, dataset[i].zk_line_item_type_id, 3, 20, zk_zero_checking);
		range_check(dataset[i].inp_os_id, dataset[i].zk_os_id, 15, 60, zk_zero_checking);
		range_check(dataset[i].inp_monetization_channel_id, dataset[i].zk_monetization_channel_id, 1, 21, zk_zero_checking);
		range_check(dataset[i].inp_ad_unit_id, dataset[i].zk_ad_unit_id, 5050, 5443, zk_zero_checking);
		range_check(dataset[i].inp_total_impressions, dataset[i].zk_total_impressions, 0, 14452, zk_zero_checking);
		range_check(dataset[i].inp_total_revenue, dataset[i].zk_total_revenue, 0, 839762, zk_zero_checking);
		range_check(dataset[i].inp_viewable_impressions, dataset[i].zk_viewable_impressions, 0, 7392, zk_zero_checking);
		range_check(dataset[i].inp_measurable_impressions, dataset[i].zk_measurable_impressions, 0, 13756, zk_zero_checking);
	}

	if(party == ALICE) {
		cout << "finished all the range checks, used OT triples = " << ZKFpExec::zk_exec->print_total_triple() << endl;
	}

	uint64_t total_impressions_trimming_sum = 0;
	uint64_t total_impressions_trimming_count = 0;

	IntFp zk_total_impressions_trimming_sum = IntFp(0, PUBLIC);
	IntFp zk_total_impressions_trimming_count = IntFp(0, PUBLIC);

	for(int i = 0; i < dataset.size(); i++) {
		trimmed_sum(dataset[i].inp_total_impressions, dataset[i].zk_total_impressions, 0, 7226, 14452, total_impressions_trimming_sum, zk_total_impressions_trimming_sum, total_impressions_trimming_count, zk_total_impressions_trimming_count, zk_zero_checking);
	}

	if(party == ALICE) {
		cout << "trimmed mean currently counts those <50% of the max " << endl;
		cout << "for total impressions, " << total_impressions_trimming_count << " out of " << dataset.size() << " satisfy this" << endl;
	}

	uint64_t total_impressions_trimmed_mean = 0;
	IntFp zk_total_impressions_trimmed_mean = IntFp(0, PUBLIC);

	finalize_trimmed_mean(total_impressions_trimming_sum, zk_total_impressions_trimming_sum, dataset.size(), total_impressions_trimming_count, zk_total_impressions_trimming_count, total_impressions_trimmed_mean, zk_total_impressions_trimmed_mean,  100, zk_zero_checking);

	if(party == ALICE) {
		cout << "the trimmed mean for total impressions = " << total_impressions_trimmed_mean * 0.01 << endl;
	}

	if(party == ALICE) {
		cout << "finished the trimmed mean checking, used OT triples = " << ZKFpExec::zk_exec->print_total_triple() << endl;
	}

	// mean
	precomputed_dataset.push_back(total_impressions_trimmed_mean);
	zk_precomputed_dataset.push_back(zk_total_impressions_trimmed_mean);

	// our data = all the user inputs (26577 * 15)
	// 			+ trimming mean (1)

	// compute the random linear combination
	// temporarily, we set the challenge to be 7

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

	batch_reveal_check_zero(zk_zero_checking.data(), zk_zero_checking.size());
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
