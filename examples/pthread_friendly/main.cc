
/*
 * MIT License
 *
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
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <slankdev/cpuset.h>
#include <dpdk/dpdk.h>
#include <thread>

size_t num1 = 1;
size_t num2 = 2;
size_t num3 = 3;
bool running = true;

int fff() {
  slankdev::cpuset cpuset;
  while (running) {
    uint64_t ret = cpuset.get();
    size_t lcore_id = rte_lcore_id();
    // printf("ffff lcore%zd ret=%lu\n", lcore_id, ret);
    // sleep(1);
  }
}

int test(void* arg)
{
  slankdev::cpuset cpuset;
  size_t* n = (size_t*)arg;
  while (running) {
    uint64_t ret = cpuset.get();
    printf("test arg=%zd lcore%u ret=%lu\n", *n, rte_lcore_id(), ret);
    sleep(1);
  }
}

int main(int argc, char** argv)
{
  dpdk::dpdk_boot(argc, argv);

  dpdk::rte_eal_remote_launch(test, &num1, 2);
  dpdk::rte_eal_remote_launch(test, &num2, 3);
  std::thread t(fff);

  getchar();
  running = false;
  t.join();
}


