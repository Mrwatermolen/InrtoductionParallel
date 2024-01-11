#include <vector>

#include "helper.h"
#include "homework.h"
#include "mpi.h"

int main(int argc, char **argv) {
  int my_rank = 0;
  int size = 1;
  int tag = 0;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  using TaskType = bin::BinTask<double, std::vector<double>, std::size_t,
                                std::vector<std::size_t>>;
  TaskType task;
  TaskType send_task;

  TaskType::BinTaskInfo info;

  if (my_rank == 0) {
    send_task = TaskType::createFromInput();
    // for (auto &&s : send_task.data()) {
    //   std::cout << s << "\n";
    // }
    task = send_task;
    // info = task.info();
  }

  // MPI_Bcast(&info, sizeof(info), MPI_CHAR, 0, MPI_COMM_WORLD);
  // std::cout << "1-=======-\n";
  // task._info = info;
  // task._bin_maxes = TaskType::binMaxesFromInfo(info);
  // task._bin_count = std::vector<std::size_t>(info._bin_n);
  // task.resetCount();

  // std::vector<int> send_counts(size);
  // for (int i = 0; i < size; ++i) {
  //   std::size_t start = 0;
  //   std::size_t end = 0;
  //   distributeTask(i, size, info.n(), &start, &end);
  //   send_counts[i] = (end - start) * sizeof(double);
  // }

  // task.info().n() = send_counts[my_rank] / sizeof(double);

  // task._data = std::vector<double>(task.info().n());

  // std::vector<int> send_displs(size);
  // send_displs[0] = 0;
  // for (int i = 1; i < size; ++i) {
  //   send_displs[i] = send_displs[i - 1] + send_counts[i - 1];
  // }
  // // print info
  // std::cout << ("process " + std::to_string(my_rank) + " send data: " +
  //               task.info().toString() + "\n");

  // MPI_Scatterv(send_task.data().data(), send_counts.data(), send_displs.data(),
  //              MPI_CHAR, task.data().data(), send_counts[my_rank], MPI_CHAR, 0,
  //              MPI_COMM_WORLD);

  // if (my_rank == 0) {
  //   MPI_Send(send_task.data().data() + send_displs[1], send_counts[1],
  //   MPI_CHAR,
  //            1, tag, MPI_COMM_WORLD);
  //   MPI_Send(send_task.data().data() + send_displs[2], send_counts[2],
  //   MPI_CHAR,
  //            2, tag, MPI_COMM_WORLD);
  // } else {
  //   MPI_Status status;
  //   MPI_Recv(task.data().data(), send_counts[my_rank], MPI_CHAR, 0, tag,
  //            MPI_COMM_WORLD, &status);
  // }

  std::cout << ("process " + std::to_string(my_rank) + " received data: " +
                task.info().toString() + "\n" + task.headDataToString() + "\n");

  MPI_Barrier(MPI_COMM_WORLD);
  // bin::mpiImp(my_rank, size, tag, task);
  MPI_Finalize();
  return 0;
}

namespace bin {
void mpiImp(int my_rank, int size, int tag,
            BinTask<double, std::vector<double>, std::size_t,
                    std::vector<std::size_t>> &task) {}
};  // namespace bin
