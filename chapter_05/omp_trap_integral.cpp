#include <omp.h>

#include <chrono>
#include <iostream>

#include "helper.h"
#include "homework.h"

int main(int argc, char **argv) {
  auto num_threads = getArgNumThread(argc, argv);
  if (num_threads == 0) {
    return 0;
  }

  using DataType = trap_integral::TrapIntegralDataTypeImp;
  using TaskType = trap_integral::TrapIntegralTaskImp;
  using ResultType = trap_integral::TrapIntegralResultImp;

  auto input_task = TaskType::createFromInput();
  auto omp_task = input_task;
  auto serial_task = input_task;

  std::cout << "Task Info: ==============================\n";
  std::cout << input_task.toString() << "\n";

  auto p = PerformanceCompare{num_threads};
  auto omp_res = ResultType{p.executeParallel(trap_integral::ompImp,
                                              num_threads, std::ref(omp_task))};
  auto serial_res = ResultType{
      p.executeSerial(trap_integral::serialImp<DataType, std::size_t,
                                               DataType(const DataType &)>,
                      serial_task.l(), serial_task.r(), serial_task.n(),
                      trap_integral::givenFuncDerivative<const double &>)};

  std::cout << ("Result: ==============================\n");
  std::cout << ("OpenMP Result: " + omp_res.toString() + "\n");
  std::cout << ("Serial Result: " + serial_res.toString() + "\n");

  std::cout << "Verify OpenMP Result: ==============================\n";
  std::cout << "Same: " << std::boolalpha
            << (decltype(omp_res)::sameResult(omp_res, serial_res)) << "\n";

  std::cout << ("Performance: ==============================\n");
  std::cout << (p.toString() + "\n");
  return 0;
}

namespace trap_integral {

TrapIntegralDataTypeImp ompImp(int num_threads,
                               const TrapIntegralTaskImp &task) {
  using DataType = TrapIntegralDataTypeImp;
  using TaskType = TrapIntegralTaskImp;
  using ResultType = TrapIntegralResultImp;

  DataType res = 0;

#pragma omp parallel num_threads(num_threads)
  {
    auto my_rank = omp_get_thread_num();
    auto size = omp_get_num_threads();

    auto h = (task.r() - task.l()) / task.n();
    std::size_t local_l;
    std::size_t local_r;
    auto n = task.n();
    distributeTask(my_rank, size, n, &local_l, &local_r);
    auto local_res =
        serialImp(task.l() + local_l * h, task.l() + local_r*h, local_r - local_l,
                  [](DataType x) { return givenFuncDerivative(x); });

#pragma omp critical
    { res += local_res; }
  }

  return res;
}

} // namespace trap_integral
