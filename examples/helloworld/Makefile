
LIB = $(HOME)/git/libdpdk_cpp
include $(LIB)/mk/dpdk.cpp.mk

CXXFLAGS = $(DPDK_CXXFLAGS) -I$(LIB)
LDFLAGS  = $(DPDK_LDFLAGS)

all:
	$(CXX) $(CXXFLAGS) main.cc $(LDFLAGS)

