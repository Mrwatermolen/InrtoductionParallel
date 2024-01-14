#include "helper.h"
#include "homework.h"
#include "homework/histogram_bin.h"
#include <chrono>
#include <cstddef>
#include <mpi.h>
#include <sstream>
#include <string>
#include <vector>

namespace homework::histogram_bin {
SizeTypeArrayImp mpiImp(int my_rank, int size, const TaskTypeImp &task,
                        const DataTypeArrayImp &data, bool printDataCopyTime) {
  std::chrono::high_resolution_clock::time_point start;
  std::chrono::high_resolution_clock::time_point end;
  SizeTypeArrayImp total_bin_count;

  TaskTypeImp local_task;

  if (my_rank == 0) {
    total_bin_count = SizeTypeArrayImp(task.binN(), 0);
    local_task = task;
  }

  if (my_rank == 0 && printDataCopyTime) {
    start = std::chrono::high_resolution_clock::now();
  }

  MPI_Bcast(&local_task, local_task.bytes(), MPI_CHAR, 0, MPI_COMM_WORLD);

  std::vector<int> send_counts(size);
  for (int i = 0; i < size; ++i) {
    std::size_t start = 0;
    std::size_t end = 0;
    distributeTask(i, size, local_task.n(), &start, &end);
    send_counts[i] = (end - start) * sizeof(DataTypeImp);
  }
  local_task.n() = send_counts[my_rank] / sizeof(DataTypeImp);

  auto &&bin_maxes = TaskTypeImp::binMaxesFromInfo(
      local_task.binN(), local_task.binMin(), local_task.binMax());

  auto local_data = DataTypeArrayImp(local_task.n(), 0);
  std::vector<int> send_displs(size);
  send_displs[0] = 0;
  for (int i = 1; i < size; ++i) {
    send_displs[i] = send_displs[i - 1] + send_counts[i - 1];
  }
  MPI_Scatterv(data.data(), send_counts.data(), send_displs.data(), MPI_CHAR,
               local_data.data(), send_counts[my_rank], MPI_CHAR, 0,
               MPI_COMM_WORLD);
  

  if (my_rank == 0 && printDataCopyTime) {
    end = std::chrono::high_resolution_clock::now();
    std::stringstream ss;
    ss << "MPI Data copy time: "
       << std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
              .count()
       << " ms\n";
    std::cout << ss.str();
  }

  SizeTypeArrayImp local_bin_count(local_task.binN(), 0);
  serialImp(local_data, static_cast<std::size_t>(0), local_data.size(),
            local_bin_count, bin_maxes, local_task.binN(), local_task.binMin());
 
  //  int count = total_task.binN(); error: count must be the same for
  // all? TODO: is right?
  int count = local_task.binN();
  MPI_Reduce(local_bin_count.data(), total_bin_count.data(), count,
             MPI_UNSIGNED_LONG, MPI_SUM, 0, MPI_COMM_WORLD); // UNSIGNED_LONG is SizeTypeImp

  return total_bin_count;
}
} // namespace homework::histogram_bin
