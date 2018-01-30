
LIB = $(HOME)/git/libdpdk_cpp
include $(LIB)/mk/dpdk.cpp.mk

CXXFLAGS = $(DPDK_CXXFLAGS) -I$(LIB) -Wno-format-security
LDFLAGS  = $(DPDK_LDFLAGS)

.c.o:
	@echo "CC $@"
	@$(CC) $(CFLAGS) $(DPDK_CXXFLAGS) -c $< -o $@

CSRC = rte_eth_pipe.c
COBJ = $(CSRC:.c=.o)
CXXSRC = main.cc
CXXOBJ = $(CXXSRC:.cc=.o)
OBJ = $(COBJ) $(CXXOBJ)

all: $(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) $(LDFLAGS)

dum:
	echo slank

clean:
	rm -rf *.o a.out

