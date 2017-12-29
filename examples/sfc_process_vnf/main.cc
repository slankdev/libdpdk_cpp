
#include <stdio.h>
#include <unistd.h>
#include <dpdk/wrap.h>
#include <thread>

class vport {
  std::string name;
  rte_ring* rx;
  rte_ring* tx;
  const bool lookup;
 public:
  vport(const char* n, bool lu=false) : name(n), rx(nullptr), tx(nullptr), lookup(lu)
  {
    const std::string rxname = name + "RX";
    const std::string txname = name + "TX";
    if (lookup) {
      rx = rte_ring_lookup(rxname.c_str());
      tx = rte_ring_lookup(txname.c_str());
      if (!rx || !tx)
        throw dpdk::exception("ring_alloc");
    } else {
      rx = dpdk::ring_alloc(rxname.c_str(), 8192);
      tx = dpdk::ring_alloc(txname.c_str(), 8192);
      if (!rx || !tx)
        throw dpdk::exception("ring_alloc");
    }
  }
  virtual ~vport()
  {
    rte_ring_free(rx);
    rte_ring_free(tx);
  }
  size_t tx_burst(rte_mbuf* const mbufs[], size_t n_mbufs)
  {
    rte_ring* txring = lookup?rx:tx;
    size_t ret = rte_ring_mp_enqueue_burst(txring, (void**)mbufs, n_mbufs, nullptr);
    return ret;
  }
  size_t rx_burst(rte_mbuf* mbufs[], size_t n_mbufs)
  {
    rte_ring* rxring = lookup?tx:rx;
    size_t ret = rte_ring_mc_dequeue_burst(rxring, (void**)mbufs, n_mbufs, nullptr);
    return ret;
  }
  void dump() const
  {
    rte_ring_dump(stdout, rx);
    rte_ring_dump(stdout, tx);
  }
};
vport* vports[2];

int l2fwd(void*)
{
  printf("start %s \n", __func__);
  const size_t n_ports = 2;
  while (true) {
    for (size_t pid=0; pid<n_ports; pid++) {
      constexpr size_t BURSTSZ = 32;
      rte_mbuf* mbufs[BURSTSZ];

      size_t nb_deq = vports[pid]->rx_burst(mbufs, BURSTSZ);
      // printf("nb_deq: %zd \n", nb_deq);
      if (nb_deq != 0) {
        printf("rx from vport%zd \n", pid);
        size_t nb_send = vports[pid^1]->tx_burst(mbufs, nb_deq);
        printf("tx to vport%zd \n", pid^1);
        if (nb_send < nb_deq) {
          dpdk::rte_pktmbuf_free_bulk(&mbufs[nb_send], nb_deq-nb_send);
        }
      }
    }
  }
}

int main(int argc, char** argv)
{
  dpdk::dpdk_boot(argc, argv);

  vports[0] = new vport("vport0", true);
  vports[1] = new vport("vport1", true);

  rte_eal_remote_launch(l2fwd, nullptr, 2);
  rte_eal_mp_wait_lcore();
}

