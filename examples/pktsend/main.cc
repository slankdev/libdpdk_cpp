
#include <stdio.h>
#include <dpdk/wrap.h>

uint8_t pkt[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x08, 0x06, 0x00, 0x01,
    0x08, 0x00, 0x06, 0x04, 0x00, 0x01, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00,
    0x00, 0x00,
};

inline void sendpkt(size_t pid, size_t qid, const uint8_t* buf, size_t size,
    rte_mempool* mp)
{
  rte_mbuf* mbuf = rte_pktmbuf_alloc(mp);
  uint8_t* ptr = rte_pktmbuf_mtod(mbuf, uint8_t*);
  memcpy(ptr, buf, size);
  mbuf->pkt_len = size;
  mbuf->data_len = size;

  rte_pktmbuf_dump(stdout, mbuf, mbuf->pkt_len);
  rte_eth_tx_burst(pid, qid, &mbuf, 1);
}

int main(int argc, char** argv)
{
  dpdk::dpdk_boot(argc, argv);
  printf("n_ports: %u\n", rte_eth_dev_count());

  struct rte_eth_conf port_conf;
  dpdk::init_portconf(&port_conf);
  struct rte_mempool* mp = dpdk::mp_alloc("RXMBUFMP", 0, 8192);
  size_t n_ports = rte_eth_dev_count();
  if (n_ports != 1) throw dpdk::exception("n-dev is not 1");
  dpdk::port_configure(0, 1, 1, &port_conf, mp);

  sendpkt(0, 0, pkt, sizeof(pkt), mp);
}



