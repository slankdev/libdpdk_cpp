
LIB = $(HOME)/git/libdpdk_cpp
include $(LIB)/mk/dpdk.cpp.mk

CXXFLAGS = $(DPDK_CXXFLAGS) -I$(LIB) -Wno-format-security -std=c++11
LDFLAGS  = $(DPDK_LDFLAGS)

.c.o:
	@echo "CC $@"
	@$(CC) $(CFLAGS) $(DPDK_CXXFLAGS) -c $< -o $@ -g -O0

CSRC = rte_eth_pipe.c
COBJ = $(CSRC:.c=.o)
CXXSRC = nfvi.cc vnf.cc
CXXOBJ = $(CXXSRC:.cc=.o)
OBJ = $(COBJ) $(CXXOBJ)

all: $(OBJ)
	$(CXX) $(CXXFLAGS) nfvi.o rte_eth_pipe.o -o nfvi.out $(LDFLAGS)
	$(CXX) $(CXXFLAGS) vnf.o rte_eth_pipe.o -o vnf.out $(LDFLAGS)

clean:
	rm -rf *.o *.out

runi:
	sudo ./nfvi.out -m 1024,1024 --proc-type=primary

runv:
	sudo ./vnf.out -m 1024,1024 --proc-type=secondary

gdbi:
	sudo gdb --args ./nfvi.out -m 1024,1024 --proc-type=primary

gdbv:
	sudo gdb --args ./vnf.out -m 1024,1024 --proc-type=secondary

