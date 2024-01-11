#include <vector>

#include "helper.h"
#include "homework.h"
#include "mpi.h"

int main(int argc, char **argv) {
  int my_rank = 0;
  int size = 1;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  using TaskType = bin::BinTask<double, std::vector<double>, std::size_t,
                                std::vector<std::size_t>>;

  TaskType total_task;

  TaskType local_task;
  TaskType::BinTaskInfo local_info;

  if (my_rank == 0) {
    total_task = TaskType::createFromInput();
    local_task = total_task;
    local_info = local_task.info();
  }

  MPI_Bcast(&local_info, sizeof(local_info), MPI_CHAR, 0, MPI_COMM_WORLD);
  local_task.info() = local_info;
  local_task.binMaxes() = TaskType::binMaxesFromInfo(local_info);
  local_task.binCount() = std::vector<std::size_t>(local_info._bin_n);
  local_task.resetCount();

  std::vector<int> send_counts(size);
  for (int i = 0; i < size; ++i) {
    std::size_t start = 0;
    std::size_t end = 0;
    distributeTask(i, size, local_info.n(), &start, &end);
    send_counts[i] = (end - start) * sizeof(double);
  }
  local_task.info().n() = send_counts[my_rank] / sizeof(double);
  local_task.data() = std::vector<double>(local_task.info().n());
  std::vector<int> send_displs(size);
  send_displs[0] = 0;
  for (int i = 1; i < size; ++i) {
    send_displs[i] = send_displs[i - 1] + send_counts[i - 1];
  }
  MPI_Scatterv(total_task.data().data(), send_counts.data(), send_displs.data(),
               MPI_CHAR, local_task.data().data(), send_counts[my_rank],
               MPI_CHAR, 0, MPI_COMM_WORLD);

  MPI_Barrier(MPI_COMM_WORLD);

  bin::serialImp(local_task.data(), local_task.info().n(),
                 local_task.binCount(), local_task.binMaxes(),

                 local_task.info().n(), local_task.info().binMin());

  // int count = total_task.info().binN(); error: count must be the same for all? TODO: is right?
  int count = local_task.info().binN();

  MPI_Reduce(local_task.binCount().data(), total_task.binCount().data(), count,
             MPI_UNSIGNED_LONG, MPI_SUM, 0, MPI_COMM_WORLD);

  if (my_rank == 0) {
    std::cout << ("process " + std::to_string(my_rank) +
                  " received data: " + total_task.info().toString() + "\n" +
                  total_task.headDataToString() + "\n" +
                  total_task.resToString() + "\n");
  }

  MPI_Finalize();
  return 0;
}

