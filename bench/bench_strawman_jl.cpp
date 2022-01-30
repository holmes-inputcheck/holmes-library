#include "histogram_nominal_check.hpp"
#include "host_ip.hpp"
#include "random_linear_combination.hpp"

int port, party;
const int threads = 32;

// number of dimensions
int num_dimensions = 5;

// the size of each dimension
int size_of_each_dimension = 10; // for simplicity, we assume each dimension has the same size

// the k parameter for JL, which is related to accuracy
int k = 6;

// number of entries to go through the range checks
// set this number to be high enough so offline phase batching pattern does not affect too much
int num_records = 1000;

typedef struct _data_entry {
	vector<uint64_t> inp;
	vector<IntFp> zk_inp;
} data_entry;

vector<data_entry> create_random_dataset(vector<uint64_t> &precomputed_dataset, vector<IntFp> &zk_precomputed_dataset) {
	// randomly generate a dataset where all elements are all in the right ranges

	vector<data_entry> dataset;

	for(int i = 0; i < num_records; i++) {
		data_entry new_entry;

		for (int j = 0; j < num_dimensions; j++) {
			// since 1 is always in the range, we use 1 in the benchmark
			new_entry.inp.emplace_back(1);
			new_entry.zk_inp.emplace_back(IntFp(1, ALICE));

			precomputed_dataset.push_back(new_entry.inp[j]);
			zk_precomputed_dataset.push_back(new_entry.zk_inp[j]);
		}
		
		dataset.push_back(new_entry);
	}

	return dataset;
}

int main(int argc, char** argv) {
	parse_party_and_port(argv, &party, &port);
	BoolIO<NetIO>* ios[threads+1];
	for(int i = 0; i < threads+1; ++i)
		ios[i] = new BoolIO<NetIO>(new NetIO(party == ALICE?nullptr:BENCH_HOST_IP,port+i), party==ALICE);

	std::cout << std::endl << "------------ strawman JL check bench ------------" << std::endl << std::endl;

	{
		FILE * fp = fopen("./benchmark_input.txt", "r");
		fscanf(fp, "%d%d%d%d", &num_dimensions, &size_of_each_dimension, &k, &num_records);
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

	/*
	 * For the naive JL, the first is to do an useless counting on each of the dimension
	 * This is to simulate the cost of one-hot encoding on each the dimension
	 */
	vector<vector<uint64_t>> counts;
	vector<vector<IntFp>> zk_counts;
	for(int i = 0; i < num_dimensions; i++) {
		counts.push_back(vector<uint64_t>());
		zk_counts.push_back(vector<IntFp>());

		for (int j = 0; j < size_of_each_dimension; j++) {
			counts[i].push_back(0);
			zk_counts[i].push_back(IntFp(0, PUBLIC));
		}
	}

	for(int i = 0; i < dataset.size(); i++) {
		for(int j = 0; j < num_dimensions; j++) {
			histogram_nominal_check(dataset[i].inp[j], dataset[i].zk_inp[j], 1, size_of_each_dimension, counts[j], zk_counts[j], zk_zero_checking);
		}
	}

	/*
	 * Then the second step is to compute that big dimension (here 10^5)
	 * for each row/entry of the data
	 */
	IntFp test_bit = IntFp(1, ALICE);
	uint64_t big_dimension_size = 1;
	for(int j = 0; j < num_dimensions; j++) {
		big_dimension_size *= size_of_each_dimension;
	}
	for(int i = 0; i < num_records; i++) {
		for(int ii = 0; ii < big_dimension_size; ii++) {
			IntFp this_bit = IntFp(1, PUBLIC);
			for (int j = 0; j < num_dimensions; j++) {
				this_bit = this_bit * test_bit;
			}
		}
	}

	// compute A * a for each entry
	for(int i = 0; i < dataset.size(); i++) {
		for(int j = 0; j < k; j++) {
			IntFp sum = IntFp(0, PUBLIC);

			uint64_t random = 123456;
			uint64_t cur = 1;

			for (int ii = 0; ii < big_dimension_size; ii++) {
				sum = sum + test_bit * cur;
				cur = cur * random;
			}
		}
	}

	if(party == ALICE) {
		cout << "after all the strawman JL projections, used OT triples = " << ZKFpExec::zk_exec->print_total_triple() << endl;
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
