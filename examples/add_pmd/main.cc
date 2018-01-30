
#include <stdio.h>
#include <unistd.h>
#include <dpdk/wrap.h>

extern "C" {
#include "rte_eth_pipe.h"
}

inline void sendpkt(size_t pid, size_t qid,
    const uint8_t* buf, size_t size, rte_mempool* mp)
{
  rte_mbuf* mbuf = rte_pktmbuf_alloc(mp);
  uint8_t* ptr = rte_pktmbuf_mtod(mbuf, uint8_t*);
  memcpy(ptr, buf, size);
  mbuf->pkt_len = size;
  mbuf->data_len = size;
  rte_pktmbuf_dump(stdout, mbuf, mbuf->pkt_len);
  rte_eth_tx_burst(pid, qid, &mbuf, 1);
}


template <size_t T>
int polling_port(void*) {
  size_t pid = T;
  printf("call %s<%zd>() on lcore%u\n", __func__, pid, rte_lcore_id());
  while (true) {
    rte_mbuf* mbufs[32];
    size_t nrx = rte_eth_rx_burst(pid, 0, mbufs, 32);
    if (nrx == 0) continue;
    printf("SLSNKSKDFJDLF port%zd recv\n", pid);
    exit(-1);
    dpdk::rte_pktmbuf_free_bulk(mbufs, nrx);
  }
}

int main(int argc, char** argv)
{
  dpdk::dpdk_boot(argc, argv);

  dpdk::eth_dev_attach("net_pipe0");
  // dpdk::eth_dev_attach("net_pipe1,attach=net_pipe0");
  printf("n_ports: %u\n", rte_eth_dev_count());

  struct rte_eth_conf port_conf;
  dpdk::init_portconf(&port_conf);
  struct rte_mempool* mp = dpdk::mp_alloc("RXMBUFMP", 0, 8192);

  constexpr size_t n_rxq = 8;
  constexpr size_t n_txq = 8;
  const size_t n_ports = rte_eth_dev_count();
  if (n_ports == 0) throw dpdk::exception("no ethdev");
  printf("%zd ports found \n", n_ports);
  for (size_t i=0; i<n_ports; i++) {
    dpdk::port_configure(i, n_rxq, n_txq, &port_conf, mp);
  }

  rte_eal_remote_launch(polling_port<0>, nullptr, 1);
  rte_eal_remote_launch(polling_port<1>, nullptr, 2);

  sleep(3);
  uint8_t pkt[] = {
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x08, 0x06, 0x00, 0x01,
      0x08, 0x00, 0x06, 0x04, 0x00, 0x01, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00,
      0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00,
      0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00,
      0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00,
  };
  sendpkt(0, 0, pkt, sizeof(pkt), mp);
  rte_eal_mp_wait_lcore();
}



