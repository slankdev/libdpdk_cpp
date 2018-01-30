
LIB = $(HOME)/git/libdpdk_cpp
include $(LIB)/mk/dpdk.cpp.mk

CXXFLAGS = $(DPDK_CXXFLAGS) -I$(LIB) -Wno-format-security
LDFLAGS  = $(DPDK_LDFLAGS)

all:
	$(CXX) $(CXXFLAGS) main.cc $(LDFLAGS)


run:
	sudo ./a.out \
		--proc-type=auto -m 1024,1024 --no-pci \
		--vdev=virtio_user0,path=/var/run/usvhost0 \
		--file-prefix=container

rum:
		# --vdev=virtio_user1,path=/var/run/usvhost1 \
