#include "jl_projector.hpp"
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

	std::cout << std::endl << "------------ JL check bench ------------" << std::endl << std::endl;

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
		cout << "start to create fake data and input them" << endl;
	}

	auto dataset = create_random_dataset(precomputed_dataset, zk_precomputed_dataset);

	if(party == ALICE) {
		cout << "num of entries: " << dataset.size() << endl;
		cout << "finished loading the dataset" << endl;
	}

	vector<uint64_t> limit;
	{
		for(int i = 0; i < num_dimensions; i++) {
			limit.push_back(size_of_each_dimension);
		}
	}
	auto info = jl_projector_init(k, limit);
	
	vector<uint64_t> res_projected;
	vector<IntFp> zk_res_projected;
	for(int i = 0; i < k; i++) {
		res_projected.push_back(0);
		zk_res_projected.push_back(IntFp(0, PUBLIC));
	}

	bool* prepared_bits[k];
	uint64_t* prepared_witnesses[k];

	for(int i = 0; i < k; i++) {
		prepared_bits[i] = new bool[dataset.size()];
		prepared_witnesses[i] = new uint64_t[dataset.size()];
	}

	const uint64_t QNR = 7;

	#pragma omp parallel for default(shared)
	for(int i = 0; i < dataset.size(); i++) {
		fmpz_t fT;
		fmpz_t a;
		fmpz_init(fT);
		fmpz_init(a);

		uint64_t j = 0;
		for(int beta = 0; beta < num_dimensions; beta ++) {
			j = add_mod(j, mult_mod(dataset[i].inp[beta], info.limit[beta]));
		}

		for(int l = 0; l < k; l++) {
			uint64_t j_cubic = mult_mod(j, j);
			j_cubic = mult_mod(j_cubic, j);

			uint64_t val = info.keys_0[l];
			val += mult_mod(info.keys_1[l], j);
			val = mod(val);
			val += mult_mod(info.keys_2[l], mult_mod(j, j));
			val = mod(val);
			val += mult_mod(info.keys_3[l], j_cubic);
			val = mod(val);

			fmpz_set_ui(fT, val);
			uint64_t inp_a;
			// find the square root of fT (or the square root of QNR * fT)
			bool is_qr = fmpz_jacobi(fT, info.p) == 1;
			if(is_qr) {
				if (fmpz_sqrtmod(a, fT, info.p) == 1) {
					inp_a = fmpz_get_ui(a); // Input witness for the quadratic residue of f(T)*b + n * f(T) * (1-b) mod p
				} else {
					cerr << "Error with computing square root of f(T)" << endl;
					exit(1);
				}
			} else {
				fmpz_mul_ui(fT, fT, QNR);
				if (fmpz_sqrtmod(a, fT, info.p) == 1) {
					inp_a = fmpz_get_ui(a); // Input witness for the quadratic residue of f(T)*b + n * f(T) * (1-b) mod p
				} else {
					cerr << "Error with computing square root of n * f(T)" << endl;
					exit(1);
				}
			}

			prepared_bits[l][i] = is_qr;
			prepared_witnesses[l][i] = inp_a;
		}

		fmpz_clear(fT);
		fmpz_clear(a);
	}

	for(int i = 0; i < dataset.size(); i++) {
		IntFp zk_j = IntFp(0, PUBLIC);
		for(int beta = 0; beta < num_dimensions; beta ++) {
			zk_j = zk_j + dataset[i].zk_inp[beta] * info.limit[beta];
		}

		vector<uint64_t> res;
		vector<IntFp> zk_res;
		for(int alpha = 0; alpha < info.k; alpha++) {
			res.push_back(0);
			zk_res.push_back(IntFp(0, PUBLIC));
		}

		for(int alpha = 0; alpha < info.k; alpha++) {
			jl_query_prf_with_prepared_data(alpha, zk_j, info, i, prepared_bits, prepared_witnesses, res[alpha], zk_res[alpha], zk_zero_checking);
		}

		for(int alpha = 0; alpha < info.k; alpha++) {
			res_projected[alpha] = add_mod(res_projected[alpha], res[alpha]);
			zk_res_projected[alpha] = zk_res_projected[alpha] + zk_res[alpha];
		}
	}

	for(int i = 0; i < k; i++) {
		delete[] prepared_bits[i];
		delete[] prepared_witnesses[i];
	}

	if(party == ALICE) {
		cout << "finished all the JL projections" << endl;
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
