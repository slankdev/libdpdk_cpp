
LIB = $(HOME)/git/libdpdk_cpp
include $(LIB)/mk/dpdk.cpp.mk

CXXFLAGS = -I$(LIB) $(DPDK_CXXFLAGS) -std=c++11
LDFLAGS  = $(DPDK_LDFLAGS)

all:
	$(CXX) $(CXXFLAGS) main.cc $(LDFLAGS)
