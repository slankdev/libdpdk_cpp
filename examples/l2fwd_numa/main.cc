
#include <thread>
#include <vector>
#include <stdio.h>
#include <unistd.h>
#include <dpdk/wrap.h>


class Thread {
  void impl()
  {
    while (true) {
      const size_t n_ports = conf.ports.size();
      for (size_t i=0; i<n_ports; i++) {
        auto& port = conf.ports[i];
        const size_t n_queues = port.queues.size();
        for (size_t j=0; j<n_queues; j++) {
          rte_mbuf* mbufs[32];
          auto& queue = port.queues[j];
          size_t n_recv = rte_eth_rx_burst(port.pid, queue.qid, mbufs, 32);
          if (n_recv == 0) continue;
          size_t n_send = rte_eth_tx_burst(port.pid^1, queue.qid, mbufs, n_recv);
          if (n_send < n_recv)
            dpdk::rte_pktmbuf_free_bulk(&mbufs[n_send], n_recv-n_send);
        }
      }
    } /* while(true) */
  }
  static int launcher(void* t)
  {
    Thread* thread = reinterpret_cast<Thread*>(t);
    thread->impl();
  }
 public:
  struct config {
    struct port {
      struct queue {
        size_t qid;
        queue(size_t id) : qid(id) {}
      };
      size_t pid;
      std::vector<queue> queues;
      port(size_t id, std::vector<queue> qs) : pid(id), queues(qs) {}
    };
    std::vector<port> ports;
    config(std::vector<port> ps) : ports(ps) {}
  };

  size_t lcore_id;
  config conf;
  Thread(size_t lid, config c) : lcore_id(lid), conf(c) {}
  void launch() { rte_eal_remote_launch(launcher, this, lcore_id); }
};


int main(int argc, char** argv)
{
  dpdk::dpdk_boot(argc, argv);
  struct rte_eth_conf port_conf;
  dpdk::init_portconf(&port_conf);
  port_conf.rxmode.mq_mode = ETH_MQ_RX_RSS;
  port_conf.rx_adv_conf.rss_conf.rss_key = NULL;
  port_conf.rx_adv_conf.rss_conf.rss_hf = ETH_RSS_IP|ETH_RSS_TCP|ETH_RSS_UDP;
  struct rte_mempool* mp[1];
  mp[0] = dpdk::mp_alloc("RXMBUFMP0", 0, 8192);
  size_t n_ports = rte_eth_dev_count();
  printf("%zd ports found \n", n_ports);
  dpdk::port_configure(0, 2, 2, &port_conf, mp[0]);
  dpdk::port_configure(1, 2, 2, &port_conf, mp[0]);

  Thread thrd(1, Thread::config({
      Thread::config::port(0, {
          Thread::config::port::queue(0),
          Thread::config::port::queue(1)
      }),
      Thread::config::port(1, {
          Thread::config::port::queue(0),
          Thread::config::port::queue(1)
      })
  }));

  thrd.launch();
  rte_eal_mp_wait_lcore();
}


