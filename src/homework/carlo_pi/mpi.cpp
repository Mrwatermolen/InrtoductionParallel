#include <cstddef>
#include <mpi.h>

#include "helper.h"
#include "homework/carlo_pi.h"

namespace homework::carlo_pi {

std::size_t mpiImp(int my_rank, int size, const TaskTypeImp &task,
                   bool printDataCopyTime) {
  using TaskType = TaskTypeImp;

  std::size_t total_counter = 0;
  std::size_t local_counter = 0;

  TaskType local_task;

  if (my_rank == 0) {
    local_task = task;
  }

  MPI_Bcast(&local_task, sizeof(TaskType), MPI_BYTE, 0, MPI_COMM_WORLD);

  std::size_t l;
  std::size_t r;
  distributeTask(my_rank, size, local_task.n(), &l, &r);
  auto n = r - l;

  local_counter = serialImp(n, local_task.radius());

  MPI_Reduce(&local_counter, &total_counter, 1,
             MPI_UNSIGNED_LONG, MPI_SUM, 0, MPI_COMM_WORLD);

  return total_counter;
}

} // namespace homework::carlo_pi