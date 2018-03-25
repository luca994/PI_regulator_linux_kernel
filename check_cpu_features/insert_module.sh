#!/bin/bash
# use sudo to execute this script

insmod check_features.ko
rmmod check_features
dmesg
