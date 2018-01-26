#!/bin/sh

QEMU=qemu-system-x86_64
HDAPATH=/home/slank/qemu/vm0.img
SOCKPATH=/tmp/sock0
HPPATH=/dev/hugepages


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
	-chardev socket,id=char1,path=$SOCKPATH \
	-netdev type=vhost-user,id=net1,chardev=char1,vhostforce,queues=1 \
	-device virtio-net-pci,mac=02:cd:c6:a0:01:01,netdev=net1,mrg_rxbuf=off

	# -netdev  type=vhost-user,id=net1,chardev=char1,vhostforce,queues=2 \
	# -device  virtio-net-pci,mac=02:cd:c6:a0:01:01,netdev=net1,mq=on,vectors=6
	# -device virtio-net-pci,mac=02:cd:c6:a0:01:01,netdev=net1

	# -chardev socket,id=char1,path=$SOCKPATH \
	# -netdev type=vhost-user,id=net1,chardev=char1,vhostforce,queues=1 \
	# -device virtio-net-pci,mac=02:cd:c6:a0:01:01,netdev=net1,mq=on,vectors=4

	# -chardev socket,id=char1,path=$SOCKPATH \
	# -netdev type=vhost-user,id=net1,chardev=char1,vhostforce,queues=1 \
	# -device virtio-net-pci,netdev=net1,mac=02:cd:c6:22:22:22,mrg_rxbuf=off
