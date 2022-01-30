#include "host_ip.hpp"

#include "bench_dataset_2.hpp"
#include "range_check.hpp"
#include "mean_check.hpp"
#include "variance_check.hpp"
#include "random_linear_combination.hpp"
#include "histogram_nominal_check.hpp"

int port, party;
const int threads = 32;

int main(int argc, char** argv) {
	parse_party_and_port(argv, &party, &port);
	BoolIO<NetIO>* ios[threads+1];
	for(int i = 0; i < threads+1; ++i)
		ios[i] = new BoolIO<NetIO>(new NetIO(party == ALICE?nullptr:BENCH_HOST_IP,port+i), party==ALICE);

	std::cout << std::endl << "------------ dataset 2 ------------" << std::endl << std::endl;

	auto total_time_start = clock_start();

	setup_zk_bool<BoolIO<NetIO>>(ios, threads, party);
	setup_zk_arith<BoolIO<NetIO>>(ios, threads, party);

	vector<IntFp> zk_zero_checking;
	vector<uint64_t> precomputed_dataset;
	vector<IntFp> zk_precomputed_dataset;

	/************************************************************************************/

	int num_of_records = 407064;

	if(party == ALICE) {
		cout << "start to load the dataset, used OT triples = " << ZKFpExec::zk_exec->print_total_triple() << endl;
	}

	auto dataset = load_dataset(precomputed_dataset, zk_precomputed_dataset, num_of_records);

	if(party == ALICE) {
		cout << "num of entries: " << dataset.size() << endl;
		cout << "finished loading the dataset, used OT triples = " << ZKFpExec::zk_exec->print_total_triple() << endl;
	}

	vector<uint64_t> counts;
	vector<IntFp> zk_counts;
	for(int i = 0; i <= 72; i++) {
		counts.push_back(0);
		zk_counts.push_back(IntFp(0, PUBLIC));
	}

	for(int i = 0; i < dataset.size(); i++) {
		range_check(dataset[i].inp_patient_nbr, dataset[i].zk_patient_nbr, 1, 71518, zk_zero_checking);
		range_check(dataset[i].inp_race, dataset[i].zk_race, 0, 5, zk_zero_checking);
		range_check(dataset[i].inp_gender, dataset[i].zk_gender, 1, 3, zk_zero_checking);
		range_check(dataset[i].inp_age, dataset[i].zk_age, 1, 10, zk_zero_checking);
		range_check(dataset[i].inp_weight, dataset[i].zk_weight, 0, 9, zk_zero_checking);
		range_check(dataset[i].inp_admission_type_id, dataset[i].zk_admission_type_id, 1, 8, zk_zero_checking);
		range_check(dataset[i].inp_discharge_disposition_id, dataset[i].zk_discharge_disposition_id, 1, 28, zk_zero_checking);
		range_check(dataset[i].inp_admission_source_id, dataset[i].zk_admission_source_id, 1, 25, zk_zero_checking);
		range_check(dataset[i].inp_time_in_hospital, dataset[i].zk_time_in_hospital, 1, 14, zk_zero_checking);
		range_check(dataset[i].inp_payer_code, dataset[i].zk_payer_code, 0, 17, zk_zero_checking);
		range_check(dataset[i].inp_num_lab_procedures, dataset[i].zk_num_lab_procedures, 1, 132, zk_zero_checking);
		range_check(dataset[i].inp_num_procedures, dataset[i].zk_num_procedures, 0, 6, zk_zero_checking);
		range_check(dataset[i].inp_num_medications, dataset[i].zk_num_medications, 1, 81, zk_zero_checking);
		range_check(dataset[i].inp_number_outpatient, dataset[i].zk_number_outpatient, 0, 42, zk_zero_checking);
		range_check(dataset[i].inp_number_emergency, dataset[i].zk_number_emergency, 0, 76, zk_zero_checking);
		range_check(dataset[i].inp_number_inpatient, dataset[i].zk_number_inpatient, 0, 21, zk_zero_checking);
		range_check(dataset[i].inp_diag_1, dataset[i].zk_diag_1, 0, 716, zk_zero_checking);
		range_check(dataset[i].inp_diag_2, dataset[i].zk_diag_2, 0, 748, zk_zero_checking);
		range_check(dataset[i].inp_diag_3, dataset[i].zk_diag_3, 0, 789, zk_zero_checking);
		range_check(dataset[i].inp_number_diagnoses, dataset[i].zk_number_diagnoses, 1, 16, zk_zero_checking);
		range_check(dataset[i].inp_max_glu_serum, dataset[i].zk_max_glu_serum, 0, 4, zk_zero_checking);
		range_check(dataset[i].inp_a1cresult, dataset[i].zk_a1cresult, 0, 3, zk_zero_checking);
		range_check(dataset[i].inp_metformin, dataset[i].zk_metformin, 0, 3, zk_zero_checking);
		range_check(dataset[i].inp_repaglinide, dataset[i].zk_repaglinide, 0, 3, zk_zero_checking);
		range_check(dataset[i].inp_nateglinide, dataset[i].zk_nateglinide, 0, 3, zk_zero_checking);
		range_check(dataset[i].inp_chlorpropamide, dataset[i].zk_chlorpropamide, 0, 3, zk_zero_checking);
		range_check(dataset[i].inp_glimepiride, dataset[i].zk_glimepiride, 0, 3, zk_zero_checking);
		range_check(dataset[i].inp_acetohexamide, dataset[i].zk_acetohexamide, 0, 3, zk_zero_checking);
		range_check(dataset[i].inp_glipizide, dataset[i].zk_glipizide, 0, 3, zk_zero_checking);
		range_check(dataset[i].inp_glyburide, dataset[i].zk_glyburide, 0, 3, zk_zero_checking);
		range_check(dataset[i].inp_tolbutamide, dataset[i].zk_tolbutamide, 0, 3, zk_zero_checking);
		range_check(dataset[i].inp_pioglitazone, dataset[i].zk_pioglitazone, 0, 3, zk_zero_checking);
		range_check(dataset[i].inp_rosiglitazone, dataset[i].zk_rosiglitazone, 0, 3, zk_zero_checking);
		range_check(dataset[i].inp_acarbose, dataset[i].zk_acarbose, 0, 3, zk_zero_checking);
		range_check(dataset[i].inp_miglitol, dataset[i].zk_miglitol, 0, 3, zk_zero_checking);
		range_check(dataset[i].inp_troglitazone, dataset[i].zk_troglitazone, 0, 3, zk_zero_checking);
		range_check(dataset[i].inp_tolazamide, dataset[i].zk_tolazamide, 0, 3, zk_zero_checking);
		range_check(dataset[i].inp_examide, dataset[i].zk_examide, 0, 3, zk_zero_checking);
		range_check(dataset[i].inp_citoglipton, dataset[i].zk_citoglipton, 0, 3, zk_zero_checking);
		range_check(dataset[i].inp_insulin, dataset[i].zk_insulin, 0, 3, zk_zero_checking);
		range_check(dataset[i].inp_glyburide_metformin, dataset[i].zk_glyburide_metformin, 0, 3, zk_zero_checking);
		range_check(dataset[i].inp_glipizide_metformin, dataset[i].zk_glipizide_metformin, 0, 3, zk_zero_checking);
		range_check(dataset[i].inp_glimepiride_pioglitazone, dataset[i].zk_glimepiride_pioglitazone, 0, 3, zk_zero_checking);
		range_check(dataset[i].inp_metformin_rosiglitazone, dataset[i].zk_metformin_rosiglitazone, 0, 3, zk_zero_checking);
		range_check(dataset[i].inp_metformin_pioglitazone, dataset[i].zk_metformin_pioglitazone, 0, 3, zk_zero_checking);
		range_check(dataset[i].inp_change, dataset[i].zk_change, 0, 3, zk_zero_checking);
		range_check(dataset[i].inp_diabetesmed, dataset[i].zk_diabetesmed, 0, 3, zk_zero_checking);
		range_check(dataset[i].inp_readmitted, dataset[i].zk_readmitted, 0, 3, zk_zero_checking);
	}

	if(party == ALICE) {
		cout << "finished all the range checks, used OT triples = " << ZKFpExec::zk_exec->print_total_triple() << endl;
	}

	for(int i = 0; i < dataset.size(); i++) {
		histogram_nominal_check(dataset[i].inp_medical_specialty, dataset[i].zk_medical_specialty, 0, 72, counts, zk_counts, zk_zero_checking);
	}

	if(party == ALICE) {
		cout << "finished the counting, used OT triples = " << ZKFpExec::zk_exec->print_total_triple() << endl;
	}

	uint64_t num_lab_procedures_sum = 0;
	IntFp zk_num_lab_procedures_sum = IntFp(0, PUBLIC);

	for(int i = 0; i < dataset.size(); i++) {
		num_lab_procedures_sum += dataset[i].inp_num_lab_procedures;
		zk_num_lab_procedures_sum = zk_num_lab_procedures_sum + dataset[i].zk_num_lab_procedures;
	}

	uint64_t num_lab_procedures_mean = 0;
	IntFp zk_num_lab_procedures_mean = IntFp(0, PUBLIC);

	mean_check(num_lab_procedures_sum, zk_num_lab_procedures_sum, dataset.size(), num_lab_procedures_mean, zk_num_lab_procedures_mean, 100, zk_zero_checking);

	if(party == ALICE) {
		cout << "finished the mean checking, used OT triples = " << ZKFpExec::zk_exec->print_total_triple() << endl;
	}

	uint64_t num_lab_procedures_squared_sum = 0;
	IntFp zk_num_lab_procedures_squared_sum = IntFp(0, PUBLIC);

	for(int i = 0; i < dataset.size(); i++) {
		num_lab_procedures_squared_sum += dataset[i].inp_num_lab_procedures * dataset[i].inp_num_lab_procedures;
		zk_num_lab_procedures_squared_sum = zk_num_lab_procedures_squared_sum + dataset[i].zk_num_lab_procedures * dataset[i].zk_num_lab_procedures;
	}

	uint64_t num_lab_procedures_variance = 0;
	IntFp zk_num_lab_procedures_variance = IntFp(0, PUBLIC);

	// this one has a fixed point shift of 100
	variance_check(num_lab_procedures_sum, zk_num_lab_procedures_sum, num_lab_procedures_squared_sum, zk_num_lab_procedures_squared_sum, dataset.size(), num_lab_procedures_variance, zk_num_lab_procedures_variance, 100, zk_zero_checking);

	if(party == ALICE) {
		cout << "finished the variance checking, used OT triples = " << ZKFpExec::zk_exec->print_total_triple() << endl;
	}

	// Export precomputation

	// histogram for medical specialty
	for(int i = 0; i < counts.size(); i++) {
		precomputed_dataset.push_back(counts[i]);
		zk_precomputed_dataset.push_back(zk_counts[i]);
	}

	// mean and variances
	precomputed_dataset.push_back(num_lab_procedures_mean);
	zk_precomputed_dataset.push_back(zk_num_lab_procedures_mean);

	precomputed_dataset.push_back(num_lab_procedures_variance);
	zk_precomputed_dataset.push_back(zk_num_lab_procedures_variance);

	// our data = all the user inputs (101766 * 49)
	// 			+ counting (73)
	// 			+ mean (1)
	// 			+ variance (1)

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
