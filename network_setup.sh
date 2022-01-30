#!/bin/bash
sudo ./traffic_control.sh -r
sudo ./traffic_control.sh --uspeed=2Gbit --dspeed=2Gbit -d=5 172.31.0.0/16
sudo tc filter show dev ens5
sudo tc qdisc show dev ens5