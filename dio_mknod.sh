#!/bin/sh

# Script to mknod for a char device driver after
#      dynamically allocating a Major number

# Written by Scott Brookes with close reference to 
#      O'Rielly's Device Driver text, p. 47

module="dio"
device="dio"

# run insmod
sudo /sbin/insmod ./$module.ko

# remove stale nodes
sudo rm -f /dev/${device}*

# must find dynamically assigned Major number
major=$(cat /proc/devices | grep -m 1 "dio" | cut -d ' ' -f 1)

# make nodes
sudo mknod -m 666 /dev/${device}0 c $major 0
sudo mknod -m 666 /dev/${device}1 c $major 1
sudo mknod -m 666 /dev/${device}2 c $major 2
sudo mknod -m 666 /dev/${device}3 c $major 3
sudo mknod -m 666 /dev/${device}4 c $major 4

