#!/bin/sh

QEMU=qemu-system-x86_64 
MEM=1G
SOCKPATH=/tmp/sock2
HPPATH=/mnt/huge_kvm

$QEMU \
	-enable-kvm -cpu host \
	-m $MEM -object memory-backend-file,id=mem,size=$MEM,mem-path=$HPPATH,share=on \
	-mem-prealloc \
	-numa node \
	-hda u1604.qcow2 \
	-boot c -vnc :0,password -monitor stdio \
	\
	-chardev socket,id=chr0,path=$SOCKPATH \
	-netdev vhost-user,id=net0,chardev=chr0,vhostforce,queues=1 \
	-device virtio-net-pci,netdev=net0


