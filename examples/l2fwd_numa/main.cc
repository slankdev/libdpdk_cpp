
#include <thread>
#include <vector>
#include <stdio.h>
#include <unistd.h>
#include <dpdk/wrap.h>


class Thread {
 private:
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
          size_t n_recv = rte_eth_rx_burst(port.pid,
                            queue.qid, mbufs, 32);
          if (n_recv == 0) continue;

#define DELAY
#ifdef DELAY
          for (size_t k=0; k<n_recv; k++) {
            size_t n = 0;
            for (size_t i=0; i<100; i++) n ++;
          }
#endif

          size_t n_send = rte_eth_tx_burst(port.pid^1,
                            queue.qid, mbufs, n_recv);
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
        void dump(FILE* fp) const
        {
          fprintf(fp, "        queue: { queue_id: %zd }\n", qid);
        }
      };
      size_t pid;
      std::vector<queue> queues;
      port(size_t id, std::vector<queue> qs) : pid(id), queues(qs) {}
      void dump(FILE* fp) const
      {
        fprintf(fp, "    port: {\n");
        fprintf(fp, "      port_id: %zd\n", pid);
        fprintf(fp, "      queues: {\n");
        for (size_t i=0; i<queues.size(); i++) {
          queues[i].dump(fp);
        }
        fprintf(fp, "      }\n");
        fprintf(fp, "    }\n");
      }
    };
    std::vector<port> ports;
    config(std::vector<port> ps) : ports(ps) {}
    void dump(FILE* fp) const
    { for (size_t i=0; i<ports.size(); i++) ports[i].dump(fp); }
  };
 private:
  config conf;
  size_t lcore_id;
 public:
  Thread(size_t lid, config c) : lcore_id(lid), conf(c) {}
  void launch() { rte_eal_remote_launch(launcher, this, lcore_id); }
  void dump(FILE* fp) const
  {
    fprintf(fp, "thread: {\n");
    fprintf(fp, "  lcore_id: %zd \n", lcore_id);
    fprintf(fp, "  ports: {\n");
    conf.dump(fp);
    fprintf(fp, "  }\n");
    fprintf(fp, "}\n");
  }

};


int main(int argc, char** argv)
{
  dpdk::dpdk_boot(argc, argv);
  struct rte_mempool* mp[1];
  mp[0] = dpdk::mp_alloc("RXMBUFMP0", 0, 8192);

  struct rte_eth_conf port_conf;
  dpdk::init_portconf(&port_conf);
  port_conf.rxmode.mq_mode = ETH_MQ_RX_RSS;
  port_conf.rx_adv_conf.rss_conf.rss_key = NULL;
  port_conf.rx_adv_conf.rss_conf.rss_hf = ETH_RSS_IP|ETH_RSS_TCP|ETH_RSS_UDP;

  size_t n_ports = rte_eth_dev_count();
  printf("%zd ports found \n", n_ports);
  dpdk::port_configure(0, 2, 2, &port_conf, mp[0]);
  dpdk::port_configure(1, 2, 2, &port_conf, mp[0]);

  Thread thrd1(1, Thread::config({
      Thread::config::port(0, {
          Thread::config::port::queue(0),
          Thread::config::port::queue(1)
      })
  }));
  Thread thrd3(3, Thread::config({
      Thread::config::port(1, {
          Thread::config::port::queue(0),
          Thread::config::port::queue(1)
      })
  }));

  thrd1.dump(stdout); thrd1.launch();
  thrd3.dump(stdout); thrd3.launch();
  rte_eal_mp_wait_lcore();
}


