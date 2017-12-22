
#include <stdio.h>
#include <unistd.h>
#include <dpdk/wrap.h>
#include <thread>

constexpr size_t n_queues = 1;
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
				size_t nb_send = rte_eth_tx_burst(pid^1, qid, mbufs, nb_recv);
        if (nb_send < nb_recv) {
          dpdk::rte_pktmbuf_free_bulk(&mbufs[nb_send], nb_recv-nb_send);
        }
			}
    }
  }
}

void debug(const rte_mempool* mp)
{
  while (true) {
    dpdk::mp_dump(mp);
    printf("-----------\n");
    sleep(1);
  }
}

int main(int argc, char** argv)
{
  dpdk::dpdk_boot(argc, argv);
  struct rte_eth_conf port_conf;
  dpdk::init_portconf(&port_conf);
  struct rte_mempool* mp = dpdk::mp_alloc("RXMBUFMP", 0, 8192);

  size_t n_ports = rte_eth_dev_count();
  if (n_ports == 0) throw dpdk::exception("no ethdev");
  printf("%zd ports found \n", n_ports);
  for (size_t i=0; i<n_ports; i++) {
    dpdk::port_configure(i, n_queues, n_queues, &port_conf, mp);
  }

  rte_eal_remote_launch(l2fwd, nullptr, 1);
  std::thread t(debug, mp);
  rte_eal_mp_wait_lcore();
  t.join();
}

