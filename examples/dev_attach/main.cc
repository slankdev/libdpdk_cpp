
#include <stdio.h>
#include <dpdk/wrap.h>


int main(int argc, char** argv)
{
  int wargc = 2;
  char* wargv[wargc];
  char argv0[] = "a.out";
  char argv1[] = "-w 0000:00:00.0";
  wargv[0] = argv0;
  wargv[1] = argv1;
  dpdk::dpdk_boot(wargc, wargv);
  printf("n_ports: %u\n", rte_eth_dev_count());

  size_t ret;
  ret = dpdk::eth_dev_attach("0000:3b:00.0");
  ret = dpdk::eth_dev_attach("0000:3b:00.1");
  ret = dpdk::eth_dev_attach("0000:86:00.0");
  ret = dpdk::eth_dev_attach("0000:86:00.1");
  printf("n_ports: %u\n", rte_eth_dev_count());
}



