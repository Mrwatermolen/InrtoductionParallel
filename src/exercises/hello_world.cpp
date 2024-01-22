#include "exercises/hello_world.h"

#include <mpi.h>
#include <omp.h>

#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include "helper.h"

namespace exercises::hello_world {

void mpiPrintHelloWorld() {
  int my_rank = 0;
  int size = 1;

  MPI_Init(nullptr, nullptr);
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  std::cout << (processInfoPrefixString(my_rank, size) +
                helloWorldString(my_rank) + "\n");

  MPI_Finalize();
}

void ompPrintHelloWorld() {
  auto num_threads = getArgNumThread(1, nullptr);

#pragma omp parallel num_threads(num_threads)
  {
    int my_rank = omp_get_thread_num();
    std::cout << (processInfoPrefixString(my_rank, num_threads) +
                  helloWorldString(my_rank) + "\n");
  }
}

void threadPrintHelloWorld() {
  auto num_threads = getArgNumThread(1, nullptr);

  std::vector<std::thread> threads;
  threads.reserve(num_threads);
  for (int i = 0; i < num_threads; ++i) {
    threads.emplace_back([i, num_threads]() {
      std::cout << (processInfoPrefixString(i, num_threads) +
                    helloWorldString(i) + "\n");
    });
  }

  for (auto& thread : threads) {
    thread.join();
  }
}

}  // namespace exercises::hello_world
