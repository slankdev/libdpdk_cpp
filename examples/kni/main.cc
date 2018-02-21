
#include <thread>
#include <stdio.h>
#include <dpdk/wrap.h>


inline struct rte_kni* kni_alloc(size_t pid, const char* devname, rte_mempool* mp)
{
  struct rte_kni_conf conf;
  memset(&conf, 0, sizeof(conf));
  snprintf(conf.name, RTE_KNI_NAMESIZE, "%s", devname);
  conf.group_id = (uint16_t)pid;
  conf.mbuf_size = 2048;

  struct rte_kni_ops ops;
  struct rte_eth_dev_info dev_info;
  memset(&ops, 0, sizeof(ops));
  memset(&dev_info, 0, sizeof(dev_info));
  rte_eth_dev_info_get(pid, &dev_info);
  conf.addr = dev_info.pci_dev->addr;
  conf.id = dev_info.pci_dev->id;
  ops.port_id = pid;

  struct rte_kni* kni = rte_kni_alloc(mp, &conf, &ops);
  if (!kni) throw dpdk::exception("rte_kni_alloc: Error");
  return kni;
}


void main_thread(size_t pid, rte_kni* kni)
{
  while (true) {
    rte_mbuf* mbufs[32];
    size_t nrx = rte_eth_rx_burst(pid, 0, mbufs, 32);
    if (nrx != 0) {
      printf("ingress\n");
      size_t ntx = rte_kni_tx_burst(kni, mbufs, nrx);
      if (ntx < nrx) dpdk::rte_pktmbuf_free_bulk(&mbufs[ntx], nrx-ntx);
    }

    nrx = rte_kni_rx_burst(kni, mbufs, 32);
    if (nrx != 0) {
      printf("egress\n");
      size_t ntx = rte_eth_tx_burst(pid, 0, mbufs, nrx);
      if (ntx < nrx) dpdk::rte_pktmbuf_free_bulk(&mbufs[ntx], nrx-ntx);
    }

    rte_kni_handle_request(kni);
  }
}


int main(int argc, char** argv)
{
  dpdk::dpdk_boot_nopciattach(argc, argv);
  size_t pid = dpdk::eth_dev_attach("0000:3b:00.0");

  rte_kni_init(1);
  struct rte_eth_conf port_conf;
  struct rte_mempool* mp = dpdk::mp_alloc("slankdev", 0, 8192);

  dpdk::init_portconf(&port_conf);
  port_conf.rxmode.hw_strip_crc = 1;
  dpdk::port_configure(pid, 1, 1, &port_conf, mp);
  rte_kni* kni = kni_alloc(pid, "kni0", mp);

  std::thread thrd(main_thread, pid, kni);
  thrd.join();
}

