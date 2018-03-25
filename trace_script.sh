#!/bin/bash

read -p 'Choose the interval tracing time: ' interv
echo
echo > /sys/kernel/debug/tracing/trace
echo 1 > /sys/kernel/debug/tracing/events/msr/write_msr/enable
sleep $interv
cat /sys/kernel/debug/tracing/trace | grep 'write_msr: 199'
echo 0 > /sys/kernel/debug/tracing/events/msr/write_msr/enable
