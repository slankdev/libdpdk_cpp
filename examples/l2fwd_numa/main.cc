
#include <stdio.h>
#include <unistd.h>
#include <dpdk/wrap.h>
#include <thread>

constexpr size_t n_queues = 1;
constexpr size_t n_ports  = 2;
size_t sock0_ports[] = {0, 1};
size_t sock1_ports[] = {2, 3};

int l2fwd_sock0(void*)
{
  printf("%s() was called on socket%u \n", __func__, rte_socket_id());
  while (true) {
    for (size_t i=0; i<n_ports; i++) {
      constexpr size_t BURSTSZ = 32;
      rte_mbuf* mbufs[BURSTSZ];
      size_t pid = sock0_ports[i];
      size_t qid = 0;

      size_t nb_recv = rte_eth_rx_burst(pid, qid, mbufs, BURSTSZ);
      if (nb_recv == 0) continue;
      size_t nb_send = rte_eth_tx_burst(pid^1, qid, mbufs, nb_recv);
      if (nb_send < nb_recv) {
        dpdk::rte_pktmbuf_free_bulk(&mbufs[nb_send], nb_recv-nb_send);
      }
    }
  }
}

int l2fwd_sock1(void*)
{
  printf("%s() was called on socket%u \n", __func__, rte_socket_id());
  while (true) {
    for (size_t i=0; i<n_ports; i++) {
      constexpr size_t BURSTSZ = 32;
      rte_mbuf* mbufs[BURSTSZ];
      size_t pid = sock1_ports[i];
      size_t qid = 0;

      size_t nb_recv = rte_eth_rx_burst(pid, qid, mbufs, BURSTSZ);
      if (nb_recv == 0) continue;
      size_t nb_send = rte_eth_tx_burst(pid^1, qid, mbufs, nb_recv);
      if (nb_send < nb_recv) {
        dpdk::rte_pktmbuf_free_bulk(&mbufs[nb_send], nb_recv-nb_send);
      }
    }
  }
}

int main(int argc, char** argv)
{
  dpdk::dpdk_boot(argc, argv);
  struct rte_eth_conf port_conf;
  dpdk::init_portconf(&port_conf);
  port_conf.rxmode.mq_mode = ETH_MQ_RX_RSS;
  port_conf.rx_adv_conf.rss_conf.rss_key = NULL;
  port_conf.rx_adv_conf.rss_conf.rss_hf = ETH_RSS_IP|ETH_RSS_TCP|ETH_RSS_UDP;
  struct rte_mempool* mp[2];
  mp[0] = dpdk::mp_alloc("RXMBUFMP0", 0, 81920);
  mp[1] = dpdk::mp_alloc("RXMBUFMP1", 1, 81920);

  size_t n_ports = rte_eth_dev_count();
  printf("%zd ports found \n", n_ports);

  dpdk::port_configure(0, n_queues, n_queues, &port_conf, mp[0]);
  dpdk::port_configure(1, n_queues, n_queues, &port_conf, mp[0]);
  dpdk::port_configure(2, n_queues, n_queues, &port_conf, mp[1]);
  dpdk::port_configure(3, n_queues, n_queues, &port_conf, mp[1]);

  rte_eal_remote_launch(l2fwd_sock0, nullptr, 2);
  rte_eal_remote_launch(l2fwd_sock1, nullptr, 3);
  rte_eal_mp_wait_lcore();
}

