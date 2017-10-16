
#include <stdio.h>
#include <dpdk/dpdk.h>
#include <slankdev/string.h>
#include <slankdev/exception.h>
using slankdev::exception;
using slankdev::format;
constexpr size_t nb_rxqueues = 2;
constexpr size_t nb_txqueues = 2;

int packet_capture(void*)
{
  const size_t n_port = rte_eth_dev_count();
  while (true) {
    for (size_t pid=0; pid<n_port; pid++) {
      for (size_t qid=0; qid<nb_rxqueues; qid++) {
        constexpr size_t BURSTSZ = 32;
        rte_mbuf* mbufs[BURSTSZ];

        size_t nb_recv = rte_eth_rx_burst(pid, qid, mbufs, BURSTSZ);
        if (nb_recv == 0) continue;

#if 1
        rte_eth_tx_burst(pid, qid, mbufs, nb_recv);
#else
        for (size_t i=0; i<nb_recv; i++) {
          printf("length=%u port=%zd queue=%zd rss_hash=0x%08x\n",
              rte_pktmbuf_pkt_len(mbufs[i]), pid, qid, mbufs[i]->hash.rss);
          dpdk::hexdump_mbuf(stdout, mbufs[i]);
          printf("\n");
          rte_pktmbuf_free(mbufs[i]);
        }
#endif
      }
    }
  } /* while (true) */
}

int main(int argc, char** argv)
{
  dpdk::dpdk_boot(argc, argv);
  size_t n_port = rte_eth_dev_count();
  if (n_port != 1)
    throw exception(format("n_port isn't 1(%zd)", n_port));

  struct rte_mempool* mp = dpdk::mp_alloc("RXMBUFMP");
  struct rte_eth_conf port_conf;
  dpdk::init_portconf(&port_conf);
  port_conf.rxmode.mq_mode = ETH_MQ_RX_RSS;
  port_conf.rx_adv_conf.rss_conf.rss_key = NULL;
  port_conf.rx_adv_conf.rss_conf.rss_hf = ETH_RSS_IP|ETH_RSS_TCP|ETH_RSS_UDP;
  printf("%zd ports found \n", n_port);
  for (size_t i=0; i<n_port; i++) {
    dpdk::port_configure(i, nb_rxqueues, nb_txqueues, &port_conf, mp);
  }

  dpdk::rte_eal_remote_launch(packet_capture, nullptr, 2);
  rte_eal_mp_wait_lcore();
}

