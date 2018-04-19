#!/bin/bash

#for FILE in /sys/kernel/debug/tracing/options/*
#do
#    echo '0' > "${FILE}"
#done
#echo '1' > /sys/kernel/debug/tracing/options/trace_printk

echo ' ' > /sys/kernel/debug/tracing/trace
