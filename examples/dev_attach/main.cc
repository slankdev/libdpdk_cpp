
#include <stdio.h>
#include <dpdk/wrap.h>


int main(int argc, char** argv)
{
  // int wargc = 2;
  // char* wargv[wargc];
  // char argv0[] = "a.out";
  // // char argv1[] = "-w 0000:00:00.0";
  // wargv[0] = argv0;
  // wargv[1] = argv1;
  // dpdk::dpdk_boot(wargc, wargv);

  dpdk::dpdk_boot(argc, argv);

  // size_t ret;
  // ret = dpdk::eth_dev_attach("0000:3b:00.0");
  // ret = dpdk::eth_dev_attach("0000:3b:00.1");
  // ret = dpdk::eth_dev_attach("0000:86:00.0");
  // ret = dpdk::eth_dev_attach("0000:86:00.1");
  printf("n_ports: %u\n", rte_eth_dev_count());

  // struct rte_eth_conf port_conf;
  // dpdk::init_portconf(&port_conf);
  // struct rte_mempool* mp = dpdk::mp_alloc("RXMBUFMP", 0, 8192);
  //
  // constexpr size_t n_rxq = 2;
  // constexpr size_t n_txq = 2;
  // size_t n_ports = rte_eth_dev_count();
  // if (n_ports == 0) throw dpdk::exception("no ethdev");
  // printf("%zd ports found \n", n_ports);
  // for (size_t i=0; i<n_ports; i++) {
  //   dpdk::port_configure(i, n_rxq, n_txq, &port_conf, mp);
  // }
}



