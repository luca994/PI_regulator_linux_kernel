#!/bin/bash

#systemctl start thermald
modprobe intel_rapl
modprobe intel_rapl_perf
modprobe x86_pkg_temp_thermal
modprobe intel_powerclamp
modprobe toshiba_acpi
#modprobe acpi_cpufreq
cpupower frequency-set -g ondemand 
