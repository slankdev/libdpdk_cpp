
#include <stdio.h>
#include <dpdk/dpdk.h>

void analyze_packet(rte_mbuf* mbuf)
{
  printf("recv\n");
  dpdk::hexdump_mbuf(stdout, mbuf);
  printf("\n");
}

int packet_capture(void*)
{
  const size_t nb_ports = rte_eth_dev_count();
  while (true) {
    for (size_t pid=0; pid<nb_ports; pid++) {
      constexpr size_t BURSTSZ = 32;
      rte_mbuf* mbufs[BURSTSZ];

      size_t nb_recv = rte_eth_rx_burst(pid, 0, mbufs, BURSTSZ);
      if (nb_recv == 0) continue;

      for (size_t i=0; i<nb_recv; i++) {
        analyze_packet(mbufs[i]);
        rte_pktmbuf_free(mbufs[i]);
      }
    }
  }
}

int main(int argc, char** argv)
{
  dpdk::dpdk_boot(argc, argv);
  struct rte_eth_conf port_conf;
  dpdk::init_portconf(&port_conf);
  struct rte_mempool* mp = dpdk::mp_alloc("RXMBUFMP");

  size_t nb_ports = rte_eth_dev_count();
  if (nb_ports == 0) throw dpdk::exception("no ethdev");

  printf("%zd ports found \n", nb_ports);
  for (size_t i=0; i<nb_ports; i++) {
    dpdk::port_configure(i, 2, 1, &port_conf, mp);
  }

  rte_eal_remote_launch(packet_capture, nullptr, 1);
  rte_eal_mp_wait_lcore();
}

