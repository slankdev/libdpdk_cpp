
#include <stdio.h>
#include <dpdk/wrap.h>
#include <rte_log.h>

inline void print_stat()
{
  printf("master_lcore: %u \n", rte_get_master_lcore());
  printf("lcore_count : %u \n", rte_lcore_count());

  const size_t n_lcore = rte_lcore_count();
  size_t lcore_id = rte_get_master_lcore();
  for (size_t i=0; i<n_lcore; i++) {
    printf("[%zd] lcore%zd: enabled \n", i, lcore_id);
    lcore_id = rte_get_next_lcore(lcore_id, 0, 0);
  }
}

int main(int argc, char** argv)
{
  rte_log_set_global_level(RTE_LOG_EMERG);
  dpdk::dpdk_boot(argc, argv);
  const size_t n_lcore = rte_lcore_count();
  printf("n_lcore: %zd \n", n_lcore);

  print_stat();
  getchar();
}


