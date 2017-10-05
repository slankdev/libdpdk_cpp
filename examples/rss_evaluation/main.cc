
/*
 * MIT License
 *
 * Copyright (c) 2017 Susanow
 * Copyright (c) 2017 Hiroki SHIROKURA
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <stdio.h>
#include <unistd.h>
#include <dpdk/wrap.h>

class l2fwd_arg {
  ssize_t qid;
  ssize_t pid;
 public:
  l2fwd_arg() : pid(-1), qid(-1) {}
  void set(size_t p, size_t q) { pid=p; qid=q; }
  ssize_t get_pid() const { return pid; }
  ssize_t get_qid() const { return qid; }
};
l2fwd_arg l2fwd_args[RTE_MAX_LCORE];

size_t n_ports = 0;
size_t n_queues = 0;
size_t n_pkts[2][10];

int l2fwd(void*)
{
  const size_t lcore_id = rte_lcore_id();
  const size_t pid = l2fwd_args[lcore_id].get_pid();
  const size_t qid = l2fwd_args[lcore_id].get_qid();
  printf("start %s on lcore%zd pid=%zd qid=%zd \n",
                      __func__, lcore_id, pid, qid);
  while (true) {
    constexpr size_t BURSTSZ = 32;
    rte_mbuf* mbufs[BURSTSZ];
    size_t nb_recv = rte_eth_rx_burst(pid, qid, mbufs, BURSTSZ);
    if (nb_recv == 0) continue;

    n_pkts[pid][qid] += nb_recv;
    for (size_t i=0; i<nb_recv; i++) {

      size_t n = 0;
      for (size_t j=0; j<100; j++) n++;

      rte_eth_tx_burst(pid^1, qid, &mbufs[i], 1);
    }
  }
  printf("end %s pid=%zd qid=%zd \n", __func__, pid, qid);
}

int print_stats(void*)
{
  printf("\n\n");
  while (true) {
    for (size_t pid=0; pid<n_ports; pid++) {
      for (size_t qid=0; qid<n_queues; qid++) {
        printf("port%zd:%zd=%zdMpkts \n", pid, qid, n_pkts[pid][qid]/1000000);
      }
    }
    fflush(stdout);
    printf("------------------\n");
    sleep(1);
  }
}

int main(int argc, char** argv)
{
  /*
   * Boot DPDK
   */
  dpdk::dpdk_boot(argc, argv);
  n_ports = rte_eth_dev_count();
  if (n_ports != 2) {
    std::string err = dpdk::format("n_port is not 2 (current %zd)", n_ports);
    throw dpdk::exception(err);
  }


  /*
   * Configuration
   */
  n_queues = 4;
  l2fwd_args[1].set(0, 0);
  l2fwd_args[2].set(0, 1);
  l2fwd_args[3].set(0, 2);
  l2fwd_args[4].set(0, 3);

  /*
   * Port Configuration
   */
  struct rte_eth_conf port_conf;
  dpdk::init_portconf(&port_conf);
  struct rte_mempool* mp = dpdk::mp_alloc("RXMBUFMP");
  port_conf.rxmode.mq_mode = ETH_MQ_RX_RSS;
  port_conf.rx_adv_conf.rss_conf.rss_key = NULL;
  port_conf.rx_adv_conf.rss_conf.rss_hf = ETH_RSS_IP|ETH_RSS_TCP|ETH_RSS_UDP;
  printf("%zd ports found \n", n_ports);
  for (size_t i=0; i<n_ports; i++) {
    dpdk::port_configure(i, n_queues, n_queues, &port_conf, mp);
  } printf("\n");

  /*
   * Launch Threads
   */
  memset(n_pkts, 0x00, sizeof(n_pkts));
  rte_eal_remote_launch(l2fwd, nullptr, 1);
  rte_eal_remote_launch(l2fwd, nullptr, 2);
  rte_eal_remote_launch(l2fwd, nullptr, 3);
  rte_eal_remote_launch(l2fwd, nullptr, 4);
  rte_eal_remote_launch(print_stats, nullptr, 5);
  while (true) {
    getchar();
    printf("reset all stats \n");
    memset(n_pkts, 0x00, sizeof(n_pkts));
  }
  rte_eal_mp_wait_lcore();
}


