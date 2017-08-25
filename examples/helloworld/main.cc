
#include <stdio.h>
#include <dpdk/wrap.h>

int thread(void*)
{
  auto lid = rte_lcore_id();
  printf("hello world lcoreid=%u \n", lid);
}

int main(int argc, char** argv)
{
  dpdk::dpdk_boot(argc, argv);
  rte_eal_remote_launch(thread, nullptr, 1);
  rte_eal_remote_launch(thread, nullptr, 2);
  rte_eal_mp_wait_lcore();
}

