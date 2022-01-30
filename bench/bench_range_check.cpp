#include "range_check.hpp"
#include "host_ip.hpp"
#include "random_linear_combination.hpp"

int port, party;
const int threads = 32;

// number of entries to go through the range checks
// set this number to be high enough so offline phase batching pattern does not affect too much
int num_records = 1000;

// the range length [1, range_len]
int range_len = 1000;

typedef struct _data_entry {
	uint64_t inp;
	IntFp zk_inp;
} data_entry;

vector<data_entry> create_random_dataset(vector<uint64_t> &precomputed_dataset, vector<IntFp> &zk_precomputed_dataset) {
	// randomly generate a dataset where all elements are all in the right ranges

	vector<data_entry> dataset;
	for(int i = 0; i < num_records; i++) {
		data_entry new_entry;
		new_entry.inp = 1; // since 1 is always in the range, we use 1 in the benchmark
		new_entry.zk_inp = IntFp(new_entry.inp, ALICE);

		precomputed_dataset.push_back(new_entry.inp);
		zk_precomputed_dataset.push_back(new_entry.zk_inp);

		dataset.push_back(new_entry);
	}

	return dataset;
}

int main(int argc, char** argv) {
	parse_party_and_port(argv, &party, &port);
	BoolIO<NetIO>* ios[threads+1];
	for(int i = 0; i < threads+1; ++i)
		ios[i] = new BoolIO<NetIO>(new NetIO(party == ALICE?nullptr:BENCH_HOST_IP,port+i), party==ALICE);

	std::cout << std::endl << "------------ range check bench ------------" << std::endl << std::endl;

	{
		FILE * fp = fopen("./benchmark_input.txt", "r");
		fscanf(fp, "%d%d", &num_records, &range_len);
		fclose(fp);
	}

	auto total_time_start = clock_start();

	setup_zk_bool<BoolIO<NetIO>>(ios, threads, party);
	setup_zk_arith<BoolIO<NetIO>>(ios, threads, party);

	vector<IntFp> zk_zero_checking;
	vector<uint64_t> precomputed_dataset;
	vector<IntFp> zk_precomputed_dataset;

	/************************************************************************************/

	if(party == ALICE) {
		cout << "start to create fake data and input them, used OT triples = " << ZKFpExec::zk_exec->print_total_triple() << endl;
	}

	auto dataset = create_random_dataset(precomputed_dataset, zk_precomputed_dataset);

	if(party == ALICE) {
		cout << "num of entries: " << dataset.size() << endl;
		cout << "after loading the dataset, used OT triples = " << ZKFpExec::zk_exec->print_total_triple() << endl;
	}

	for(int i = 0; i < dataset.size(); i++) {
		range_check(dataset[i].inp, dataset[i].zk_inp, 1, range_len, zk_zero_checking);
	}

	if(party == ALICE) {
		cout << "after all the range checks, used OT triples = " << ZKFpExec::zk_exec->print_total_triple() << endl;
	}

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
