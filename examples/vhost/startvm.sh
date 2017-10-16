#!/bin/sh

QEMU=qemu-system-x86_64
HDAPATH=/home/slank/qemu/u1604.qcow2
SOCKPATH=/tmp/sock0

# HPPATH=???
HPPATH=/mnt/huge_kvm


# $QEMU \
numactl --physcpubind=4-7 $QEMU \
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
	-net nic,model=virtio,macaddr=52:54:00:11:11:11 \
	-net tap,script=/etc/qemu-ifup \
	\
	-chardev socket,id=chr0,path=$SOCKPATH \
	-netdev vhost-user,id=net0,chardev=chr0,vhostforce,queues=2 \
	-device virtio-net-pci,netdev=net0

