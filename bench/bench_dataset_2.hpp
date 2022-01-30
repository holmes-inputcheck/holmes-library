#ifndef EMP_ZK_BENCH_DATASET_2_H
#define EMP_ZK_BENCH_DATASET_2_H

#include "csv_reader.hpp"

typedef struct _medical_data_entry {
	uint64_t inp_patient_nbr;
	IntFp zk_patient_nbr;
	uint64_t inp_race;
	IntFp zk_race;
	uint64_t inp_gender;
	IntFp zk_gender;
	uint64_t inp_age;
	IntFp zk_age;
	uint64_t inp_weight;
	IntFp zk_weight;
	uint64_t inp_admission_type_id;
	IntFp zk_admission_type_id;
	uint64_t inp_discharge_disposition_id;
	IntFp zk_discharge_disposition_id;
	uint64_t inp_admission_source_id;
	IntFp zk_admission_source_id;
	uint64_t inp_time_in_hospital;
	IntFp zk_time_in_hospital;
	uint64_t inp_payer_code;
	IntFp zk_payer_code;
	uint64_t inp_medical_specialty;
	IntFp zk_medical_specialty;
	uint64_t inp_num_lab_procedures;
	IntFp zk_num_lab_procedures;
	uint64_t inp_num_procedures;
	IntFp zk_num_procedures;
	uint64_t inp_num_medications;
	IntFp zk_num_medications;
	uint64_t inp_number_outpatient;
	IntFp zk_number_outpatient;
	uint64_t inp_number_emergency;
	IntFp zk_number_emergency;
	uint64_t inp_number_inpatient;
	IntFp zk_number_inpatient;
	uint64_t inp_diag_1;
	IntFp zk_diag_1;
	uint64_t inp_diag_2;
	IntFp zk_diag_2;
	uint64_t inp_diag_3;
	IntFp zk_diag_3;
	uint64_t inp_number_diagnoses;
	IntFp zk_number_diagnoses;
	uint64_t inp_max_glu_serum;
	IntFp zk_max_glu_serum;
	uint64_t inp_a1cresult;
	IntFp zk_a1cresult;
	uint64_t inp_metformin;
	IntFp zk_metformin;
	uint64_t inp_repaglinide;
	IntFp zk_repaglinide;
	uint64_t inp_nateglinide;
	IntFp zk_nateglinide;
	uint64_t inp_chlorpropamide;
	IntFp zk_chlorpropamide;
	uint64_t inp_glimepiride;
	IntFp zk_glimepiride;
	uint64_t inp_acetohexamide;
	IntFp zk_acetohexamide;
	uint64_t inp_glipizide;
	IntFp zk_glipizide;
	uint64_t inp_glyburide;
	IntFp zk_glyburide;
	uint64_t inp_tolbutamide;
	IntFp zk_tolbutamide;
	uint64_t inp_pioglitazone;
	IntFp zk_pioglitazone;
	uint64_t inp_rosiglitazone;
	IntFp zk_rosiglitazone;
	uint64_t inp_acarbose;
	IntFp zk_acarbose;
	uint64_t inp_miglitol;
	IntFp zk_miglitol;
	uint64_t inp_troglitazone;
	IntFp zk_troglitazone;
	uint64_t inp_tolazamide;
	IntFp zk_tolazamide;
	uint64_t inp_examide;
	IntFp zk_examide;
	uint64_t inp_citoglipton;
	IntFp zk_citoglipton;
	uint64_t inp_insulin;
	IntFp zk_insulin;
	uint64_t inp_glyburide_metformin;
	IntFp zk_glyburide_metformin;
	uint64_t inp_glipizide_metformin;
	IntFp zk_glipizide_metformin;
	uint64_t inp_glimepiride_pioglitazone;
	IntFp zk_glimepiride_pioglitazone;
	uint64_t inp_metformin_rosiglitazone;
	IntFp zk_metformin_rosiglitazone;
	uint64_t inp_metformin_pioglitazone;
	IntFp zk_metformin_pioglitazone;
	uint64_t inp_change;
	IntFp zk_change;
	uint64_t inp_diabetesmed;
	IntFp zk_diabetesmed;
	uint64_t inp_readmitted;
	IntFp zk_readmitted;
} medical_data_entry;

vector<medical_data_entry> load_dataset(vector<uint64_t> &precomputed_dataset, vector<IntFp> &zk_precomputed_dataset, int num_records) {
	// assert(num_records <= 101766);
	assert(num_records == 407064);

	// vector<vector<uint64_t>> dataset_csv = read_csv("./bench/dataset_2.csv", num_records);
	vector<vector<uint64_t>> dataset_csv = read_csv("./bench/dataset_2.csv", 101766);

	vector<medical_data_entry> dataset;
	dataset.reserve(num_records);

	for(int k = 0; k < 4; k++) {
		for(int i = 0; i < dataset_csv.size(); i++) {
			vector<uint64_t> cur = dataset_csv[i];
			vector<IntFp> zk_cur;

			for(int j = 0; j < 49; j++) {
				precomputed_dataset.push_back(cur[j]);

				zk_cur.push_back(IntFp(cur[j], ALICE));
				zk_precomputed_dataset.push_back(zk_cur[j]);
			}

			medical_data_entry new_entry;
			new_entry.inp_patient_nbr = cur[0];
			new_entry.inp_race = cur[1];
			new_entry.inp_gender = cur[2];
			new_entry.inp_age = cur[3];
			new_entry.inp_weight = cur[4];
			new_entry.inp_admission_type_id = cur[5];
			new_entry.inp_discharge_disposition_id = cur[6];
			new_entry.inp_admission_source_id = cur[7];
			new_entry.inp_time_in_hospital = cur[8];
			new_entry.inp_payer_code = cur[9];
			new_entry.inp_medical_specialty = cur[10];
			new_entry.inp_num_lab_procedures = cur[11];
			new_entry.inp_num_procedures = cur[12];
			new_entry.inp_num_medications = cur[13];
			new_entry.inp_number_outpatient = cur[14];
			new_entry.inp_number_emergency = cur[15];
			new_entry.inp_number_inpatient = cur[16];
			new_entry.inp_diag_1 = cur[17];
			new_entry.inp_diag_2 = cur[18];
			new_entry.inp_diag_3 = cur[19];
			new_entry.inp_number_diagnoses = cur[20];
			new_entry.inp_max_glu_serum = cur[21];
			new_entry.inp_a1cresult = cur[22];
			new_entry.inp_metformin = cur[23];
			new_entry.inp_repaglinide = cur[24];
			new_entry.inp_nateglinide = cur[25];
			new_entry.inp_chlorpropamide = cur[26];
			new_entry.inp_glimepiride = cur[27];
			new_entry.inp_acetohexamide = cur[28];
			new_entry.inp_glipizide = cur[29];
			new_entry.inp_glyburide = cur[30];
			new_entry.inp_tolbutamide = cur[31];
			new_entry.inp_pioglitazone = cur[32];
			new_entry.inp_rosiglitazone = cur[33];
			new_entry.inp_acarbose = cur[34];
			new_entry.inp_miglitol = cur[35];
			new_entry.inp_troglitazone = cur[36];
			new_entry.inp_tolazamide = cur[37];
			new_entry.inp_examide = cur[38];
			new_entry.inp_citoglipton = cur[39];
			new_entry.inp_insulin = cur[40];
			new_entry.inp_glyburide_metformin = cur[41];
			new_entry.inp_glipizide_metformin = cur[42];
			new_entry.inp_glimepiride_pioglitazone = cur[43];
			new_entry.inp_metformin_rosiglitazone = cur[44];
			new_entry.inp_metformin_pioglitazone = cur[45];
			new_entry.inp_change = cur[46];
			new_entry.inp_diabetesmed = cur[47];
			new_entry.inp_readmitted = cur[48];

			new_entry.zk_patient_nbr = zk_cur[0];
			new_entry.zk_race = zk_cur[1];
			new_entry.zk_gender = zk_cur[2];
			new_entry.zk_age = zk_cur[3];
			new_entry.zk_weight = zk_cur[4];
			new_entry.zk_admission_type_id = zk_cur[5];
			new_entry.zk_discharge_disposition_id = zk_cur[6];
			new_entry.zk_admission_source_id = zk_cur[7];
			new_entry.zk_time_in_hospital = zk_cur[8];
			new_entry.zk_payer_code = zk_cur[9];
			new_entry.zk_medical_specialty = zk_cur[10];
			new_entry.zk_num_lab_procedures = zk_cur[11];
			new_entry.zk_num_procedures = zk_cur[12];
			new_entry.zk_num_medications = zk_cur[13];
			new_entry.zk_number_outpatient = zk_cur[14];
			new_entry.zk_number_emergency = zk_cur[15];
			new_entry.zk_number_inpatient = zk_cur[16];
			new_entry.zk_diag_1 = zk_cur[17];
			new_entry.zk_diag_2 = zk_cur[18];
			new_entry.zk_diag_3 = zk_cur[19];
			new_entry.zk_number_diagnoses = zk_cur[20];
			new_entry.zk_max_glu_serum = zk_cur[21];
			new_entry.zk_a1cresult = zk_cur[22];
			new_entry.zk_metformin = zk_cur[23];
			new_entry.zk_repaglinide = zk_cur[24];
			new_entry.zk_nateglinide = zk_cur[25];
			new_entry.zk_chlorpropamide = zk_cur[26];
			new_entry.zk_glimepiride = zk_cur[27];
			new_entry.zk_acetohexamide = zk_cur[28];
			new_entry.zk_glipizide = zk_cur[29];
			new_entry.zk_glyburide = zk_cur[30];
			new_entry.zk_tolbutamide = zk_cur[31];
			new_entry.zk_pioglitazone = zk_cur[32];
			new_entry.zk_rosiglitazone = zk_cur[33];
			new_entry.zk_acarbose = zk_cur[34];
			new_entry.zk_miglitol = zk_cur[35];
			new_entry.zk_troglitazone = zk_cur[36];
			new_entry.zk_tolazamide = zk_cur[37];
			new_entry.zk_examide = zk_cur[38];
			new_entry.zk_citoglipton = zk_cur[39];
			new_entry.zk_insulin = zk_cur[40];
			new_entry.zk_glyburide_metformin = zk_cur[41];
			new_entry.zk_glipizide_metformin = zk_cur[42];
			new_entry.zk_glimepiride_pioglitazone = zk_cur[43];
			new_entry.zk_metformin_rosiglitazone = zk_cur[44];
			new_entry.zk_metformin_pioglitazone = zk_cur[45];
			new_entry.zk_change = zk_cur[46];
			new_entry.zk_diabetesmed = zk_cur[47];
			new_entry.zk_readmitted = zk_cur[48];

			dataset.push_back(new_entry);
		}
	}

	return dataset;
}

#endif //EMP_ZK_BENCH_DATASET_2_H