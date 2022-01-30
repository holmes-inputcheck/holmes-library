#ifndef EMP_ZK_BENCH_DATASET_3_HPP
#define EMP_ZK_BENCH_DATASET_3_HPP

#include "csv_reader.hpp"

typedef struct _auctioning_data_entry {
	uint64_t inp_date;
	IntFp zk_date;
	uint64_t inp_site_id;
	IntFp zk_site_id;
	uint64_t inp_ad_type_id;
	IntFp zk_ad_type_id;
	uint64_t inp_geo_id;
	IntFp zk_geo_id;
	uint64_t inp_device_category_id;
	IntFp zk_device_category_id;
	uint64_t inp_advertiser_id;
	IntFp zk_advertiser_id;
	uint64_t inp_order_id;
	IntFp zk_order_id;
	uint64_t inp_line_item_type_id;
	IntFp zk_line_item_type_id;
	uint64_t inp_os_id;
	IntFp zk_os_id;
	uint64_t inp_monetization_channel_id;
	IntFp zk_monetization_channel_id;
	uint64_t inp_ad_unit_id;
	IntFp zk_ad_unit_id;
	uint64_t inp_total_impressions;
	IntFp zk_total_impressions;
	uint64_t inp_total_revenue;
	IntFp zk_total_revenue;
	uint64_t inp_viewable_impressions;
	IntFp zk_viewable_impressions;
	uint64_t inp_measurable_impressions;
	IntFp zk_measurable_impressions;
} auctioning_data_entry;

vector<auctioning_data_entry> load_dataset(vector<uint64_t> &precomputed_dataset, vector<IntFp> &zk_precomputed_dataset, int num_records) {
	assert(num_records <= 567291);

	vector<vector<uint64_t>> dataset_csv = read_csv("./bench/dataset_3.csv", num_records);

	vector<auctioning_data_entry> dataset;
	dataset.reserve(num_records);

	for(int i = 0; i < dataset_csv.size(); i++) {
		vector<uint64_t> cur = dataset_csv[i];
		vector<IntFp> zk_cur;

		for(int j = 0; j < 15; j++) {
			precomputed_dataset.push_back(cur[j]);

			zk_cur.push_back(IntFp(cur[j], ALICE));
			zk_precomputed_dataset.push_back(zk_cur[j]);
		}

		auctioning_data_entry new_entry;
		new_entry.inp_date = cur[0];
		new_entry.inp_site_id = cur[1];
		new_entry.inp_ad_type_id = cur[2];
		new_entry.inp_geo_id = cur[3];
		new_entry.inp_device_category_id = cur[4];
		new_entry.inp_advertiser_id = cur[5];
		new_entry.inp_order_id = cur[6];
		new_entry.inp_line_item_type_id = cur[7];
		new_entry.inp_os_id = cur[8];
		new_entry.inp_monetization_channel_id = cur[9];
		new_entry.inp_ad_unit_id = cur[10];
		new_entry.inp_total_impressions = cur[11];
		new_entry.inp_total_revenue = cur[12];
		new_entry.inp_viewable_impressions = cur[13];
		new_entry.inp_measurable_impressions = cur[14];

		new_entry.zk_date = zk_cur[0];
		new_entry.zk_site_id = zk_cur[1];
		new_entry.zk_ad_type_id = zk_cur[2];
		new_entry.zk_geo_id = zk_cur[3];
		new_entry.zk_device_category_id = zk_cur[4];
		new_entry.zk_advertiser_id = zk_cur[5];
		new_entry.zk_order_id = zk_cur[6];
		new_entry.zk_line_item_type_id = zk_cur[7];
		new_entry.zk_os_id = zk_cur[8];
		new_entry.zk_monetization_channel_id = zk_cur[9];
		new_entry.zk_ad_unit_id = zk_cur[10];
		new_entry.zk_total_impressions = zk_cur[11];
		new_entry.zk_total_revenue = zk_cur[12];
		new_entry.zk_viewable_impressions = zk_cur[13];
		new_entry.zk_measurable_impressions = zk_cur[14];

		dataset.push_back(new_entry);
	}

	return dataset;
}

#endif //EMP_ZK_BENCH_DATASET_3_HPP