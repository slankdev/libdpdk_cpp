
#pragma once

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
      rx = dpdk::ring_alloc(rxname.c_str(), 8192, 0);
      tx = dpdk::ring_alloc(txname.c_str(), 8192, 0);
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


