
#include <stdio.h>
#include <dpdk/wrap.h>

int l2fwd(void*)
{
  const size_t n_ports = rte_eth_dev_count();
  while (true) {
    for (size_t pid=0; pid<n_ports; pid++) {
      constexpr size_t BURSTSZ = 32;
      rte_mbuf* mbufs[BURSTSZ];

      size_t nb_recv = rte_eth_rx_burst(pid, 0, mbufs, BURSTSZ);
      if (nb_recv == 0) continue;
      rte_eth_tx_burst(pid^1, 0, mbufs, nb_recv);
    }
  }
}

int main(int argc, char** argv)
{
  dpdk::dpdk_boot(argc, argv);
  struct rte_eth_conf port_conf;
  dpdk::init_portconf(&port_conf);
  struct rte_mempool* mp = dpdk::mp_alloc("RXMBUFMP");

  size_t n_ports = rte_eth_dev_count();
  if (n_ports == 0) throw dpdk::exception("no ethdev");
  printf("%zd ports found \n", n_ports);
  for (size_t i=0; i<n_ports; i++) {
    dpdk::port_configure(i, 1, 1, &port_conf, mp);
  }

  rte_eal_remote_launch(l2fwd, nullptr, 1);
  rte_eal_mp_wait_lcore();
}

