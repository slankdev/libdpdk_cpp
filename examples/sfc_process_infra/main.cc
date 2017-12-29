
#include <stdio.h>
#include <unistd.h>
#include <dpdk/wrap.h>
#include <thread>

class vport {
  std::string name;
  rte_ring* rx;
  rte_ring* tx;
 public:
  vport(const char* n) : name(n), rx(nullptr), tx(nullptr)
  {
    std::string rxname = name + "RX";
    std::string txname = name + "TX";
    rx = dpdk::ring_alloc(rxname.c_str(), 8192);
    tx = dpdk::ring_alloc(txname.c_str(), 8192);
    if (!rx || !tx)
      throw dpdk::exception("ring_alloc");
  }
  virtual ~vport()
  {
    rte_ring_free(rx);
    rte_ring_free(tx);
  }
  size_t tx_burst(rte_mbuf* const mbufs[], size_t n_mbufs)
  {
    size_t ret = rte_ring_mp_enqueue_burst(tx, (void**)mbufs, n_mbufs, nullptr);
    return ret;
  }
  size_t rx_burst(rte_mbuf* mbufs[], size_t n_mbufs)
  {
    size_t ret = rte_ring_mc_dequeue_burst(rx, (void**)mbufs, n_mbufs, nullptr);
    return ret;
  }
  void dump() const
  {
    rte_ring_dump(stdout, rx);
    rte_ring_dump(stdout, tx);
  }
};

vport* vports[2];

constexpr size_t n_queues = 1;
int l2fwd(void*)
{
  printf("start %s \n", __func__);
  const size_t n_ports = rte_eth_dev_count();
  while (true) {
    for (size_t pid=0; pid<n_ports; pid++) {
			for (size_t qid=0; qid<n_queues; qid++) {
				constexpr size_t BURSTSZ = 32;
				rte_mbuf* mbufs[BURSTSZ];

				size_t nb_recv = rte_eth_rx_burst(pid, qid, mbufs, BURSTSZ);
				if (nb_recv != 0) {
          printf("rx from port%zd \n", pid);
          size_t nb_send = vports[pid]->tx_burst(mbufs, nb_recv);
          if (nb_send < nb_recv) {
            dpdk::rte_pktmbuf_free_bulk(&mbufs[nb_send], nb_recv-nb_send);
          }
        }

        size_t nb_deq = vports[pid]->rx_burst(mbufs, BURSTSZ);
        if (nb_deq != 0) {
          printf("rx %zd packets from vring%zd \n", nb_deq, pid);
          size_t nb_send = rte_eth_tx_burst(pid, qid, mbufs, nb_deq);
          rte_pktmbuf_dump(stdout, mbufs[0], 0);
          printf("tx %zd packets to port%zd\n", nb_send, pid);
          if (nb_send < nb_recv) {
            dpdk::rte_pktmbuf_free_bulk(&mbufs[nb_send], nb_recv-nb_send);
          }
        }

			}
    }
  }
}

void debug()
{
  for (size_t i=0; ; i++) {
    printf("-0x%lx--------------------------\n", i);
    vports[0]->dump();
    vports[1]->dump();
    sleep(1);
  }
}

int main(int argc, char** argv)
{
  dpdk::dpdk_boot(argc, argv);
  struct rte_eth_conf port_conf;
  dpdk::init_portconf(&port_conf);
  struct rte_mempool* mp = nullptr;
  mp = dpdk::mp_alloc("RXMBUFMP0", 0, 8192);

  const size_t n_ports = rte_eth_dev_count();
  printf("%zd ports found \n", n_ports);
  if (n_ports != 2) throw dpdk::exception("no ethdev");
  for (size_t i=0; i<n_ports; i++) {
    dpdk::port_configure(i, n_queues, n_queues, &port_conf, mp);
  }

  vports[0] = new vport("vport0");
  vports[1] = new vport("vport1");

  // std::thread t(debug);
  rte_eal_remote_launch(l2fwd, nullptr, 1);
  // t.join();
  rte_eal_mp_wait_lcore();
}

