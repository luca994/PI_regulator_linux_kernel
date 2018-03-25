#!/bin/bash

#sudo systemctl stop thermald
#sudo modprobe msr
cpupower frequency-set -g userspace
sudo rmmod -f intel_rapl
sudo rmmod -f intel_rapl_perf
sudo rmmod -f x86_pkg_temp_thermal
sudo rmmod -f intel_powerclamp
#sudo rmmod -f toshiba_acpi
sudo rmmod -f acpi_cpufreq
