
LIBSLANKDEV = $(HOME)/git/libslankdev
LIB = $(HOME)/git/libdpdk_cpp
include $(LIB)/mk/dpdk.cpp.mk

CXXFLAGS += -I$(LIBSLANKDEV)
CXXFLAGS += -I$(LIB) $(DPDK_CXXFLAGS) -std=c++11 -Wno-format-security
LDFLAGS  = $(DPDK_LDFLAGS)

all:
	$(CXX) $(CXXFLAGS) main.cc $(LDFLAGS)

SOCK=/tmp/sock0
run:
	sudo rm -f $(SOCK)
	sudo ./a.out  --vdev=net_vhost0,iface=$(SOCK),queues=1

