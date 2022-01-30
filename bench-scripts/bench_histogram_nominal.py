#!/usr/bin/env python
from common import *

configure_network()
subprocess.run(["cmake", "."])
subprocess.run(["make"])

print("running the bench_histogram_nominal")

num_records = 100000
print("running the case for " + str(num_records) + " entries")

time.sleep(5)
for i in range(5):
    num_groups = 10 + 10 * i
    write_configure_info(str(num_records) + " " + str(num_groups))
    subprocess.run(["bin/bench_histogram_nominal", os.getenv("EMP_MY_PARTY_ID"), "5000"])
    copy_benchmark_result_to_log("bench_histogram_nominal " + str(num_records) + " " + str(num_groups))
