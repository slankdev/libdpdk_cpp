
#include <stdio.h>
#include <unistd.h>
#include <dpdk/wrap.h>
#include <thread>
#include <vector>


class threadarg {
 public:
  std::vector<size_t> pid_list;
  std::vector<size_t> qid_list;
  void show() const
  {
    const size_t n_ports  = pid_list.size();
    const size_t n_queues = qid_list.size();
    printf("<");
    for (size_t i=0; i<n_ports; i++) {
      printf("%zd%s", pid_list[i], i+1<n_ports?",":"");
    }
    printf(">");
    printf(" [");
    for (size_t i=0; i<n_queues; i++) {
      printf("%zd%s", qid_list[i], i+1<n_queues?",":"");
    }
    printf("]\n");
  }
};

int l2fwd(void* ta_)
{
  threadarg* arg = reinterpret_cast<threadarg*>(ta_);
  const size_t n_ports  = arg->pid_list.size();
  const size_t n_queues = arg->qid_list.size();

  printf("call %s() sock%u ", __func__, rte_socket_id());
  arg->show();

  while (true) {
    for (size_t pi=0; pi<n_ports; pi++) {
      size_t pid = arg->pid_list[pi];
      for (size_t qi=0; qi<n_queues; qi++) {
        size_t qid = arg->qid_list[pi];

        constexpr size_t BURSTSZ = 32;
        rte_mbuf* mbufs[BURSTSZ];

        // printf("%zd:%zd \n", pid, qid);
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

void debug(struct rte_mempool* mp[])
{
  for (size_t i=0; ;i++) {
    printf("%zd \n", i);
    printf("---------------------\n");
    dpdk::mp_dump(mp[0]);
    printf("---------------------\n");
    dpdk::mp_dump(mp[1]);
    printf("=========================\n");
    sleep(1);
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

  struct rte_mempool* mp[100];
  mp[0] = dpdk::mp_alloc("RXMBUFMP0", 0, 8192);
  mp[1] = dpdk::mp_alloc("RXMBUFMP1", 1, 8192);

  const size_t n_queues = 2;
  size_t n_ports = rte_eth_dev_count();
  if (n_ports != 2) throw dpdk::exception("no ethdev");
  printf("%zd ports found \n", n_ports);
  dpdk::port_configure(0, n_queues, n_queues, &port_conf, mp[0]);
  dpdk::port_configure(1, n_queues, n_queues, &port_conf, mp[1]);

  threadarg ta0;
  ta0.pid_list = {0};
  ta0.qid_list = {0,1};
  threadarg ta1;
  ta1.pid_list = {1};
  ta1.qid_list = {0,1};

  // std::thread t0(debug, mp);
  sleep(1); rte_eal_remote_launch(l2fwd, &ta0, 2);
  sleep(1); rte_eal_remote_launch(l2fwd, &ta1, 1);

  rte_eal_mp_wait_lcore();
  // t0.join();
}

