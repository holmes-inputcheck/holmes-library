#!/usr/bin/env python
from common import *

configure_network()
subprocess.run(["cmake", "."])
subprocess.run(["make"])

print("running the bench_trimmed_mean")

fixed_point_shift = 100
num_records = 100000
print("running the case for " + str(num_records) + " entries with a fixed-point shift of " + str(fixed_point_shift))

time.sleep(5)
for i in range(5):
    range = pow(2, 8 + 4 * i)
    write_configure_info(str(num_records) + " " + str(fixed_point_shift) + " " + str(range))
    subprocess.run(["bin/bench_trimmed_mean", os.getenv("EMP_MY_PARTY_ID"), "5000"])
    copy_benchmark_result_to_log("bench_trimmed_mean " + str(num_records) + " " + str(fixed_point_shift) + " " + str(range))