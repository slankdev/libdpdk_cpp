
#include <stdio.h>
#include <unistd.h>
#include <dpdk/wrap.h>

extern "C" {
#include "rte_eth_pipe.h"
}

constexpr size_t n_rxq = 1;
constexpr size_t n_txq = 1;

size_t _dpdk[2];
size_t _pipe[2];
size_t ports[4] = {};

int polling_port(void*) {
  printf("call %s() on lcore%u\n", __func__, rte_lcore_id());
  const size_t n_port = rte_eth_dev_count();
  while (true) {
    for (size_t pid_idx=0; pid_idx<n_port; pid_idx++) {
      size_t pid = ports[pid_idx];
      for (size_t qid=0; qid<n_rxq; qid++) {
        rte_mbuf* mbufs[32];
        // if (pid==0 || pid==2) printf("dpdk rx\n");
        size_t nrx = rte_eth_rx_burst(pid, qid, mbufs, 32);
        // if (pid==0 || pid==2) printf("dpdk rx end\n");

        if (nrx == 0) continue;
        for (size_t i=0; i<nrx; i++) {

          size_t dstpid_idx;
          switch (pid_idx) {
            case 0: dstpid_idx = 2; break;
            case 1: dstpid_idx = 3; break;
            case 2: dstpid_idx = 0; break;
            case 3: dstpid_idx = 1; break;
            default: throw dpdk::exception("OKASHI");
          }
          size_t dstpid = ports[dstpid_idx];
          int ret = rte_eth_tx_burst(dstpid, qid, &mbufs[i], 1);
          if  (ret < 1) rte_pktmbuf_free(mbufs[i]);
        }
      }
    }
  }
}

int main(int argc, char** argv)
{
  dpdk::dpdk_boot_nopciattach(argc, argv);

  // _dpdk[0] = dpdk::eth_dev_attach("net_tap0,iface=tap0");

  _dpdk[0] = dpdk::eth_dev_attach("0000:3b:00.0");
  _dpdk[1] = dpdk::eth_dev_attach("0000:3b:00.1");
  _pipe[0] = dpdk::eth_dev_attach("net_pipe0");
  _pipe[1] = dpdk::eth_dev_attach("net_pipe1");
  ports[0] = _dpdk[0];
  ports[1] = _dpdk[1];
  ports[2] = _pipe[0];
  ports[3] = _pipe[1];

  // _dpdk[1] = dpdk::eth_dev_attach("net_tap1,iface=tap1");

  printf("dpdk0=%zd\n", _dpdk[0]);
  printf("dpdk1=%zd\n", _dpdk[1]);
  printf("pipe0=%zd\n", _pipe[0]);
  printf("pipe1=%zd\n", _pipe[1]);
  printf("n_ports: %u\n", rte_eth_dev_count());

  struct rte_eth_conf port_conf;
  dpdk::init_portconf(&port_conf);
  struct rte_mempool* mp = dpdk::mp_alloc("RXMBUFMP", 0, 8192);

  const size_t n_ports = rte_eth_dev_count();
  if (n_ports != 4) throw dpdk::exception("invalid n_port");
  printf("%zd ports found \n", n_ports);
  for (size_t i=0; i<n_ports; i++) {
    dpdk::port_configure(ports[i], n_rxq, n_txq, &port_conf, mp);
  }

  // getchar();
  rte_eal_remote_launch(polling_port, nullptr, 1);
  rte_eal_mp_wait_lcore();
}



