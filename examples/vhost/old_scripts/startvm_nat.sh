#!/bin/sh

QEMU=qemu-system-x86_64
HDAPATH=/home/slank/qemu/u1604.qcow2

$QEMU \
	-enable-kvm -cpu host -smp 2 \
	-m 512 \
	-hda $HDAPATH \
	-boot c -vnc :0 -monitor stdio

