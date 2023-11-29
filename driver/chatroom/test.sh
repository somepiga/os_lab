#!/bin/bash

rmmod globalvar
rm /dev/globalvar
make
insmod ./globalvar.ko
mknod /dev/globalvar c 290 0
