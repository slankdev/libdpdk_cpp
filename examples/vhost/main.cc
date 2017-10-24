

#include <stdio.h>
#include <dpdk/wrap.h>
#include <slankdev/string.h>
#include <slankdev/exception.h>
using slankdev::exception;
using slankdev::format;

size_t num[] = {0,1,2,3,4,5};
constexpr size_t n_queues = 1;
int impl(void* arg)
{
  printf("running thread on lcore%u \n", rte_lcore_id());
  const size_t n_ports = rte_eth_dev_count();
  while (true) {
    for (size_t pid=0; pid<n_ports; pid++) {
			for (size_t qid=0; qid<n_queues; qid++) {
        constexpr size_t BURSTSZ = 32;
        rte_mbuf* mbufs[BURSTSZ];
        size_t nb_recv = rte_eth_rx_burst(pid, qid, mbufs, BURSTSZ);
        if (nb_recv == 0) continue;
        rte_eth_tx_burst(pid, qid, mbufs, nb_recv);
			}
    }
  }
}

int main(int argc, char** argv)
{
  dpdk::dpdk_boot(argc, argv);

  struct rte_eth_conf port_conf;
  dpdk::init_portconf(&port_conf);
  struct rte_mempool* mp = dpdk::mp_alloc("RXMBUFMP");

  size_t n_ports = rte_eth_dev_count();
  printf("%zd ports found\n", n_ports);
  for (size_t i=0; i<n_ports; i++) {
    const size_t n_rxq = n_queues;
    const size_t n_txq = n_queues;
    dpdk::port_configure(i, n_rxq, n_txq, &port_conf, mp);
  }

  /* init vhostuser port */
  std::string devargs = format(
      "net_vhost0,iface=/tmp/sock0,queues=%zd", n_queues);
  uint8_t vhost_pid;
  int ret = rte_eth_dev_attach(devargs.c_str(), &vhost_pid);
  if (ret < 0) {
    throw dpdk::exception("rte_eth_dev_attach");
  }
  printf("vhost_pid: %u\n", vhost_pid);
  dpdk::port_configure(vhost_pid, n_queues, n_queues, &port_conf, mp);

  printf("configure done n_queue=%zd \n", n_queues);
  n_ports = rte_eth_dev_count();
  dpdk::check_all_ports_link_status(n_ports, 0b11, 5);
  printf("\n");

  rte_eal_remote_launch(impl, &num[0], 2);
  rte_eal_mp_wait_lcore();
}


