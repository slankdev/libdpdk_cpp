
#include <stdio.h>
#include <dpdk/wrap.h>
#include <rte_log.h>
#include <thread>

inline void print_stat()
{
  printf("--------------------------------\n");
  printf("execution    : lcore%u \n", rte_lcore_id());
  printf("proc-type    : %s \n",
      rte_eal_process_type()==RTE_PROC_PRIMARY?"primary":"secondary");
  printf("master_lcore : %u \n", rte_get_master_lcore());
  printf("lcore_count  : %u \n", rte_lcore_count());
  printf("RTE_MAX_LCORE: %u \n", RTE_MAX_LCORE);
  printf("\n");

  size_t lcore_id;
  RTE_LCORE_FOREACH(lcore_id) {
    printf("[%u] lcore%zd: enabled %s\n",
        rte_lcore_index(lcore_id),
        lcore_id,
        lcore_id==rte_get_master_lcore()?"master":"");
  }
  printf("--------------------------------\n");
}

bool running;
inline int function(void*)
{
  int n =0;
  running = true;
  while (running) {
    // printf("ff\n");
    n ++;
  }
  return n;
}

int main(int argc, char** argv)
{
  rte_log_set_global_level(RTE_LOG_EMERG);
  dpdk::dpdk_boot(argc, argv);
  const size_t n_lcore = rte_lcore_count();

  print_stat();
  function(nullptr);
  getchar();
  running = false;
  rte_eal_mp_wait_lcore();
}



