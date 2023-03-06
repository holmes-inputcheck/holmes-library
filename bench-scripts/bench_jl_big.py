#!/usr/bin/env python
from common import *

configure_network()
subprocess.run(["cmake", "."])
subprocess.run(["make"])

print("running the bench_jl_varying_num_dim")

k = 200
num_records_list = [100000, 200000, 500000]
dim_size_tuple = [(1, 10), (4, 10), (4, 50)]

time.sleep(5)
for num_records in num_records_list:
    print("running the case for " + str(num_records) + " entries, with k value " + str(k))
    for i in range(3):
        num_dimensions = dim_size_tuple[i][0]
        size_of_each_dimension = dim_size_tuple[i][1]
        write_configure_info(str(num_dimensions) + " " + str(size_of_each_dimension) + " " + str(k) + " " + str(num_records))
        subprocess.run(["bin/bench_jl", os.getenv("EMP_MY_PARTY_ID"), "5000"])
        copy_benchmark_result_to_log("bench_jl varying " + str(num_dimensions) + " " + str(size_of_each_dimension) + " " + str(k) + " " + str(num_records))
