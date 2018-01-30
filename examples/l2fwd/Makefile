
LIB = $(HOME)/git/libdpdk_cpp
include $(LIB)/mk/dpdk.cpp.mk

CXXFLAGS = -I$(LIB) $(DPDK_CXXFLAGS) -std=c++11 -Wno-format-security
LDFLAGS  = $(DPDK_LDFLAGS)

all:
	$(CXX) $(CXXFLAGS) main.cc $(LDFLAGS)

run:
	sudo rm -rf sock0
	sudo rm -rf sock1
	sudo ./a.out \
		--socket-mem=1024,1024 \
		--vdev=net_vhost0,iface=sock0 \
		--vdev=net_vhost1,iface=sock1 \
		--file-prefix=host --no-pci

client:
	sudo ./a.out \
		--proc-type=auto \
		--socket-mem=1025,1024 \
		--vdev=net_virtio_user0,path=sock0 \
		--vdev=net_virtio_user1,path=sock1

