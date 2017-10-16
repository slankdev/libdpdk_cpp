#!/bin/sh

QEMU=qemu-system-x86_64
HDAPATH=/var/lib/libvirt/images/vm001.img
SOCKPATH=/tmp/sock0

# HPPATH=???
HPPATH=/mnt/huge_kvm

COREMASK=0x1e

taskset $COREMASK $QEMU \
	-enable-kvm -cpu host -smp 4 \
	-hda $HDAPATH \
	\
	-m 512 \
	-object memory-backend-file,id=mem,size=512M,mem-path=$HPPATH,share=on \
	-numa node,memdev=mem -m 512 -mem-prealloc \
	\
	-boot c -vnc :0,password \
	-monitor stdio \
	\
	-chardev socket,id=chr0,path=$SOCKPATH \
	-netdev vhost-user,id=net0,chardev=chr0,vhostforce,queues=1 \
	-device virtio-net-pci,netdev=net0

