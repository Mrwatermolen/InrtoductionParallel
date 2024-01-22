#include <mpi.h>

#include <iostream>

#include "helper.h"
#include "homework/matrix_multi_vector.h"

int main(int argc, char** argv) {
  MPI_Init(&argc, &argv);

  int my_rank = 0;
  int size = 1;
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  using TaskType = homework::mmv::TaskTypeImp;
  using ResultType = homework::mmv::ResultTypeImp;

  auto mpi_f = [](int my_rank, int size, const TaskType& task) {
    return homework::mmv::mpiImp(my_rank, size, task, true);
  };

  auto serial_f = []() {};

  auto pc = PerformanceCompare{size, mpi_f, serial_f};

  TaskType task;

  if (my_rank == 0) {
    std::cout << mpiConfigureToString(size) << "\n";

    task = TaskType::createFromInput();
  }

  try {
    pc.executeParallel(my_rank, size, task);
  } catch (std::exception& e) {
    std::cout << e.what() << "\n";
    MPI_Abort(MPI_COMM_WORLD, MPI_ERR_ARG);
    return 0;
  }

  MPI_Finalize();
  return 0;
}
