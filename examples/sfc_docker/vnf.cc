
#include <stdio.h>
#include <unistd.h>
#include <dpdk/wrap.h>
#include <thread>
#include "vport.h"

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

