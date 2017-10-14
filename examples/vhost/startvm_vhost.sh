#!/bin/sh

QEMU=qemu-system-x86_64
HDAPATH=/home/slank/qemu/u1604.qcow2
SOCKPATH=/tmp/sock0
HPPATH=/mnt/huge_kvm

$QEMU \
	-enable-kvm -cpu host -smp 4 \
	-m 512 -object memory-backend-file,id=mem,size=512M,mem-path=$HPPATH,share=on \
	-numa node,memdev=mem -m 512 -mem-prealloc \
	-hda $HDAPATH \
	-boot c -vnc :0 -monitor stdio \
	\
	-chardev socket,id=chr0,path=$SOCKPATH \
	-netdev vhost-user,id=net0,chardev=chr0,vhostforce,queues=1 \
	-device virtio-net-pci,netdev=net0


