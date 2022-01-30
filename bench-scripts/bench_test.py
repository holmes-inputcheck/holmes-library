#!/usr/bin/env python
from common import *

configure_network()
subprocess.run(["cmake", "."])
subprocess.run(["make"])

print("start running benchmark of datasets test")

time.sleep(5)
subprocess.run(["bin/bench_test", os.getenv("EMP_MY_PARTY_ID"), "5000"])
