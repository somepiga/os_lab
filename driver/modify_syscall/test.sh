#!/bin/bash

rmmod modify
rm /dev/modify
make
insmod ./modify.ko
mknod /dev/modify c 291 0
