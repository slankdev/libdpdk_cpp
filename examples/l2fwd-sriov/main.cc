
#include <stdio.h>
#include <dpdk/dpdk.h>

uint8_t port0_mac[6];
uint8_t port1_mac[6];

constexpr size_t n_queues = 4;
int l2fwd(void*)
{
  const size_t n_ports = rte_eth_dev_count();
  while (true) {
    for (size_t pid=0; pid<n_ports; pid++) {
			for (size_t qid=0; qid<n_queues; qid++) {
				constexpr size_t BURSTSZ = 32;
				rte_mbuf* mbufs[BURSTSZ];

				size_t nb_recv = rte_eth_rx_burst(pid, qid, mbufs, BURSTSZ);
				if (nb_recv == 0) continue;

        for (size_t i=0; i<nb_recv; i++) {

          if (pid==0) {
            // printf("0->1\n");
            // uint8_t dst[] = { 0xa0, 0x36, 0x9f, 0x3e, 0xa0, 0x8a };
            uint8_t dst[] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
            uint8_t* p = rte_pktmbuf_mtod(mbufs[i], uint8_t*);
            memcpy(p+0, dst, 6);
            memcpy(p+6, port1_mac, 6);
          } else if (pid==1) {
            // printf("1->0\n");
            // uint8_t dst[] = { 0xa0, 0x36, 0x9f, 0x3e, 0x7b, 0x7a };
            uint8_t dst[] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
            uint8_t* p = rte_pktmbuf_mtod(mbufs[i], uint8_t*);
            memcpy(p+0, dst, 6);
            memcpy(p+6, port0_mac, 6);
          }

          rte_eth_tx_burst(pid^1, qid, &mbufs[i], 1);
        }

			}
    }
  }
}

inline std::string print_mac(uint8_t* addr)
{
  std::string s;
  for (size_t i=0; i<6; i++) {
    s += dpdk::format("%02x%s", addr[i], i<5?":":"");
  }
  return s;
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
    dpdk::port_configure(i, n_queues, n_queues, &port_conf, mp);
  }

  rte_eth_macaddr_get(0, (ether_addr*)port0_mac);
  rte_eth_macaddr_get(1, (ether_addr*)port1_mac);
  printf("port0: %s \n", print_mac(port0_mac).c_str());
  printf("port1: %s \n", print_mac(port1_mac).c_str());

  l2fwd(nullptr);
  // rte_eal_remote_launch(l2fwd, nullptr, 1);
  // rte_eal_mp_wait_lcore();
}

