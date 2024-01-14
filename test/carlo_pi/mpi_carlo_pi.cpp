#include <mpi.h>

#include <iostream>

#include "helper.h"
#include "homework/carlo_pi.h"

int main(int argc, char **argv) {
  int my_rank = 0;
  int size = 1;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  using TaskType = homework::carlo_pi::TaskTypeImp;
  using DataType = homework::carlo_pi::DataTypeImp;
  using ResultType = homework::carlo_pi::ResultTypeImp;

  TaskType input_task;
  TaskType mpi_task;
  TaskType serial_task;

  if (my_rank == 0) {
    std::cout << mpiConfigureToString(size) << "\n";
    input_task = TaskType::createFromInput();
    mpi_task = input_task;
    serial_task = input_task;

    std::cout << "Task info: ======================\n";
    std::cout << input_task.toString() << "\n";

    std::cout << "Running\n";
  }

  auto pc = PerformanceCompare{size, homework::carlo_pi::mpiImp,
                               homework::carlo_pi::serialImp<DataType>};

  auto mpi_res =
      ResultType{pc.executeParallel(my_rank, size, std::ref(mpi_task), false),
                 mpi_task.n()};

  if (my_rank == 0) {
    auto serial_res =
        ResultType{pc.executeSerial(serial_task.n(), serial_task.radius()),
                   serial_task.n()};

    std::cout << "MPI Result: ======================\n";
    std::cout << mpi_res.toString() << "\n";

    std::cout << "Serial Result: ======================\n";
    std::cout << serial_res.toString() << "\n";

    std::cout << "Verify Result: ======================\n";
    std::cout << ("Same: ") << std::boolalpha
              << (ResultType::sameResult(serial_res, mpi_res)) << "\n";

    std::cout << "Performance Compare: ==============================\n";
    std::cout << pc.toString() << "\n";
  }

  MPI_Finalize();
  return 0;
}
