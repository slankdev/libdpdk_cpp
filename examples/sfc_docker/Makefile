
LIB = $(HOME)/git/libdpdk_cpp
include $(LIB)/mk/dpdk.cpp.mk

CXXFLAGS = -I$(LIB) $(DPDK_CXXFLAGS) -std=c++11 -Wno-format-security
LDFLAGS  = $(DPDK_LDFLAGS)

all:
	$(CXX) $(CXXFLAGS) nfvi.cc -o nfvi.out $(LDFLAGS)
	$(CXX) $(CXXFLAGS) vnf.cc  -o vnf.out  $(LDFLAGS)

run:
	sudo ./nfvi.out \
		--vdev=net_tap0,iface=tap0 \
		--vdev=net_tap1,iface=tap1 \

rundumm:
	sudo rm -f /tmp/sock0
	sudo rm -f /tmp/sock1
	sudo ./a.out \
		--socket-mem=4000 \
		--vdev=net_vhost0,iface=/tmp/sock0 \
		--vdev=net_vhost1,iface=/tmp/sock1

