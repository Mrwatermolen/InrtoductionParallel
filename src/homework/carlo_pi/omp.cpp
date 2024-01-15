#include <omp.h>

#include <cstddef>

#include "helper.h"
#include "homework/carlo_pi.h"

// #ifdef _OPENMP
// #include <omp.h>
// #endif

namespace homework::carlo_pi {

std::size_t ompImp(int num_threads, const TaskTypeImp &task) {
  // #ifndef _OPENMP
  //   std::cout << "Warning: OpenMP is not enabled\n";
  // #endif

  using TaskType = TaskTypeImp;
  using DataType = DataTypeImp;

  std::size_t total_counter = 0;

  TaskType local_task;

  local_task = task;

#pragma omp parallel num_threads(num_threads)
  {
    // #ifdef _OPENMP
    //     int my_rank = omp_get_thread_num();
    // #else
    //     int my_rank = 0;
    //     num_threads = 1;
    // #endif

    int my_rank = omp_get_thread_num();

    std::size_t l;
    std::size_t r;
    distributeTask(my_rank, num_threads, local_task.n(), &l, &r);
    auto n = r - l;

    auto local_counter = serialImp(n, local_task.radius());

#pragma omp critical
    { total_counter += local_counter; }
  }

  return total_counter;
}

}  // namespace homework::carlo_pi
