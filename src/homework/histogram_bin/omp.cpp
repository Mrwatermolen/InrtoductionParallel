#include <omp.h>

#include <iostream>

#include "helper.h"
#include "homework/histogram_bin.h"

// #ifdef _OPENMP
// #include <omp.h>
// #endif

namespace homework::histogram_bin {

SizeTypeArrayImp ompImp(int num_threads, const TaskTypeImp &task,
                        const DataTypeArrayImp &data) {
  using TaskType = TaskTypeImp;
  using DataType = DataTypeImp;
  using DataTypeArray = DataTypeArrayImp;
  using SizeType = SizeTypeImp;
  using SizeTypeArray = SizeTypeArrayImp;

  SizeTypeArray bin_count(task.binN(), 0);

  // #ifndef _OPENMP
  // std::cout << "Warning: OpenMP is not enabled\n";
  // #endif

  std::cout << "Num: " << omp_get_max_threads() << "\n";
  auto bin_maxes =
      TaskType::binMaxesFromInfo(task.binN(), task.binMin(), task.binMax());

#pragma omp parallel num_threads(num_threads)
  {
    // #ifdef _OPENMP
    //     int my_rank = omp_get_thread_num();
    // #else
    //     int my_rank = 0;
    //     num_threads = 1;
    // #endif

    int my_rank = omp_get_thread_num();

    SizeType l = 0;
    SizeType r = 0;
    distributeTask(my_rank, num_threads, task.n(), &l, &r);

    auto local_bin_count = SizeTypeArray(task.binN(), 0);
    serialImp(data, l, r, local_bin_count, bin_maxes, task.binN(),
              task.binMin());

#pragma omp critical
    {
      for (SizeType i = 0; i < task.binN(); ++i) {
        bin_count[i] += local_bin_count[i];
      }
    }
  }

  return bin_count;
}

}  // namespace homework::histogram_bin
