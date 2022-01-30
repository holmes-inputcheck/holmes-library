#!/usr/bin/env python
from common import *

configure_network()
subprocess.run(["cmake", "."])
subprocess.run(["make"])

print("running the bench_histogram_numeric")

num_records = 100000
num_groups = 10

print("running the case for " + str(num_records) + " entries and " + str(num_groups) + "groups")

time.sleep(5)
for i in range(5):
    range = pow(2, 4 + 2 * i)
    write_configure_info(str(num_records) + " " + str(num_groups) + " " + str(range))
    subprocess.run(["bin/bench_histogram_numeric", os.getenv("EMP_MY_PARTY_ID"), "5000"])
    copy_benchmark_result_to_log("bench_histogram_numeric " + str(num_records) + " " + str(num_groups) + " " + str(range))