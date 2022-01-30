#!/usr/bin/env python
from common import *

configure_network()
subprocess.run(["cmake", "."])
subprocess.run(["make"])

print("running the bench_strawman_jl_varying_num_dim")

k = 41
# 41 for 100(k), 44 for 200(k)
num_records = 100
size_of_each_dimension = 10
print("running the case for " + str(num_records) + " entries, with k value " + str(k) + ", and each dimension has size " + str(size_of_each_dimension))

time.sleep(5)
for i in range(5):
    num_dimensions = 1 + i * 1
    write_configure_info(str(num_dimensions) + " " + str(size_of_each_dimension) + " " + str(k) + " " + str(num_records))
    subprocess.run(["bin/bench_strawman_jl", os.getenv("EMP_MY_PARTY_ID"), "5000"])
    copy_benchmark_result_to_log("bench_strawman_jl varying " + str(num_dimensions) + " " + str(size_of_each_dimension) + " " + str(k) + " " + str(num_records))