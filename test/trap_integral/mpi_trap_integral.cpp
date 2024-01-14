#include <mpi.h>

#include <chrono>
#include <cstddef>
#include <iostream>
#include <string>

#include "helper.h"
#include "homework/trap_integral.h"

int main(int argc, char *argv[]) {
  int my_rank = 0;
  int size = 1;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  using DataType = homework::trap_integral::TrapIntegralDataTypeImp;
  using TaskType = homework::trap_integral::TrapIntegralTaskImp;
  using ResultType = homework::trap_integral::TrapIntegralResultImp;

  TaskType input_task;

  TaskType serial_task;
  TaskType mpi_task;
  ResultType mpi_res;

  if (my_rank == 0) {
    std::cout << (mpiConfigureToString(size) + "\n");
  }

  if (my_rank == 0) {
    input_task = TaskType::createFromInput();
    serial_task = input_task;
    mpi_task = input_task;
    std::cout << "Task Info: ==============================\n";
    std::cout << (input_task.toString() + "\n");
  }

  auto p = PerformanceCompare{
      size, homework::trap_integral::mpiImp,
      homework::trap_integral::serialImp<DataType, std::size_t,
                                         DataType(const DataType &)>};
  mpi_res =
      ResultType{p.executeParallel(my_rank, size, std::ref(mpi_task), false)};

  if (my_rank == 0) {
    std::cout << ("Result: ==============================\n");
    std::cout << ("MPI Result: " + mpi_res.toString() + "\n");

    auto serial_res = ResultType{p.executeSerial(
        serial_task.l(), serial_task.r(), serial_task.n(),
        homework::trap_integral::givenFuncDerivative<const double &>)};

    std::cout << ("Serial Result: " + serial_res.toString() + "\n");

    auto exact_res = ResultType{homework::trap_integral::givenFunc(input_task.r()) -
                                homework::trap_integral::givenFunc(input_task.l())};

    std::cout << ("Exact Result: " + exact_res.toString() + "\n");

    std::cout << ("Verify MPI Result: ==============================\n");
    std::cout << ("Same: ") << std::boolalpha
              << (ResultType::sameResult(mpi_res, exact_res)) << "\n";

    std::cout << ("Performance: ==============================\n");
    std::cout << (p.toString() + "\n");
  };

  MPI_Finalize();
}
