#!/usr/bin/env python
from common import *

configure_network()
subprocess.run(["cmake", "."])
subprocess.run(["make"])

print("running the bench_range_check")

num_records_list = [100000, 200000, 500000]

for num_records in num_records_list:
    print("running the case for " + str(num_records) + " entries")
    time.sleep(5)
    for i in range(5):
        range_size = pow(2, 8 + 4 * i)
        write_configure_info(str(num_records) + " " + str(range_size))
        subprocess.run(["bin/bench_range_check", os.getenv("EMP_MY_PARTY_ID"), "5000"])
        copy_benchmark_result_to_log("bench_range_check " + str(num_records) + " " + str(range_size))
