#!/usr/bin/env python
from common import *

configure_network()
subprocess.run(["cmake", "."])
subprocess.run(["make"])

print("running the bench_jl_varying_size_dim")

k = 200
num_records_list = [100000, 200000, 500000]
num_dimensions = 4

for num_records in num_records_list:
    print("running the case for " + str(num_records) + " entries, with k value " + str(k) + ", and " + str(num_dimensions) + " dimensions")
    time.sleep(5)
    for i in range(5):
        size_of_each_dimension = 5 + 5 * i
        write_configure_info(str(num_dimensions) + " " + str(size_of_each_dimension) + " " + str(k) + " " + str(num_records))
        subprocess.run(["bin/bench_jl", os.getenv("EMP_MY_PARTY_ID"), "5000"])
        copy_benchmark_result_to_log("bench_jl " + str(num_dimensions) + " varying " + str(size_of_each_dimension) + " " + str(k) + " " + str(num_records))
