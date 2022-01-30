#!/usr/bin/env python
from common import *

configure_network()
subprocess.run(["cmake", "."])
subprocess.run(["make"])

print("running the bench_variance_check")

fixed_point_shift = 100
print("running the case for entries with a fixed-point shift of " + str(fixed_point_shift))

time.sleep(5)
for i in range(5):
    num_records = 1000000 + i * 1000000
    write_configure_info(str(num_records) + " " + str(fixed_point_shift))
    subprocess.run(["bin/bench_variance_check", os.getenv("EMP_MY_PARTY_ID"), "5000"])
    copy_benchmark_result_to_log("bench_variance_check " + str(num_records) + " " + str(fixed_point_shift))