
LIB = $(HOME)/git/libdpdk_cpp
include $(LIB)/mk/dpdk.cpp.mk

CXXFLAGS = $(DPDK_CXXFLAGS) -I$(LIB) -std=c++11 -Wno-format-security
LDFLAGS  = $(DPDK_LDFLAGS)

all:
	$(CXX) $(CXXFLAGS) main.cc $(LDFLAGS)

