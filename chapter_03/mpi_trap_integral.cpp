#include <mpi.h>

#include <chrono>
#include <cstddef>
#include <iostream>
#include <string>

#include "helper.h"
#include "homework.h"

int main(int argc, char *argv[]) {
  int my_rank = 0;
  int size = 1;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  using DataType = trap_integral::TrapIntegralDataTypeImp;
  using TaskType = trap_integral::TrapIntegralTaskImp;
  using ResultType = trap_integral::TrapIntegralResultImp;

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
    std::cout << (input_task.toString() + "\n");
  }

  auto p = PerformanceCompare{size};
  mpi_res = ResultType{p.executeParallel(trap_integral::mpiImp, my_rank, size,
                                         std::ref(mpi_task), false)};

  if (my_rank == 0) {
    std::cout << ("MPI Result: " + mpi_res.toString() + "\n");

    auto serial_res = ResultType{
        p.executeSerial(trap_integral::serialImp<DataType, std::size_t,
                                                 DataType(const DataType &)>,
                        serial_task.l(), serial_task.r(), serial_task.n(),
                        trap_integral::givenFuncDerivative<const double &>)};

    std::cout << ("Serial Result: " + serial_res.toString() + "\n");

    auto exact_res = ResultType{trap_integral::givenFunc(input_task.r()) -
                                trap_integral::givenFunc(input_task.l())};

    std::cout << ("Exact Result: " + exact_res.toString() + "\n");

    std::cout << ("Verify MPI Result: ==============================\n");
    std::cout << ("Same: ") << std::boolalpha
              << (ResultType::sameResult(mpi_res, exact_res)) << "\n";

    std::cout << ("Performance: ==============================\n");
    std::cout << (p.toString() + "\n");
  };

  MPI_Finalize();
}

namespace trap_integral {

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

}  // namespace trap_integral
