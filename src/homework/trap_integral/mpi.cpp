#include "helper.h"
#include "homework.h"
#include "homework/trap_integral.h"
#include <chrono>
#include <cstddef>
#include <mpi.h>
#include <sstream>
#include <string>
#include <vector>

namespace homework::trap_integral {
TrapIntegralDataTypeImp mpiImp(int my_rank, int size, TrapIntegralTaskImp &task,
                               bool printDataCopyTime) {
  using DataType = TrapIntegralDataTypeImp;
  using TaskType = TrapIntegralTaskImp;

  std::chrono::high_resolution_clock::time_point start;
  std::chrono::high_resolution_clock::time_point end;

  start = std::chrono::high_resolution_clock::now();

  DataType total_res{};
  DataType local_res{};

  MPI_Bcast(&task, task.bytes(), MPI_CHAR, 0, MPI_COMM_WORLD);

  if (my_rank == 0 && printDataCopyTime) {
    end = std::chrono::high_resolution_clock::now();
    std::cout << "MPI Task Data Copy Time: "
              << std::chrono::duration_cast<std::chrono::milliseconds>(end -
                                                                       start)
                     .count()
              << " ms\n";
  }

  auto h = (task.r() - task.l()) / task.n();
  std::size_t l = 0;
  std::size_t r = 0;
  distributeTask(my_rank, size, task.n(), &l, &r);
  auto local_task = TaskType{task.l() + l * h, task.l() + r * h, r - l};

  auto f = [&]() {
    return serialImp(
        local_task.l(), local_task.r(), local_task.n(),
        [](DataType x) { return trap_integral::givenFuncDerivative(x); });
  };

  local_res = f();

  MPI_Reduce(&local_res, &total_res, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

  return total_res;
}
} // namespace homework::trap_integral
