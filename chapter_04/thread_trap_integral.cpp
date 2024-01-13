#include <numeric>
#include <thread>
#include <vector>

#include "helper.h"
#include "homework.h"

int main(int argc, char **argv) {
  auto num_threads = getArgNumThread(argc, argv);
  if (num_threads == 0) {
    return 0;
  }

  std::cout << threadConfigureToString(num_threads) << "\n";

  using DataType = trap_integral::TrapIntegralDataTypeImp;
  using TaskType = trap_integral::TrapIntegralTaskImp;
  using ResultType = trap_integral::TrapIntegralResultImp;

  auto input_task = trap_integral::TrapIntegralTask<double>::createFromInput();
  auto thread_task = input_task;
  auto serial_task = input_task;

  std::cout << "Task Info: ==============================\n";
  std::cout << input_task.toString() << "\n";

  auto p = PerformanceCompare{num_threads};
  auto thread_res = ResultType{p.executeParallel(
      trap_integral::threadImp, num_threads, std::ref(thread_task))};
  auto serial_res = ResultType{
      p.executeSerial(trap_integral::serialImp<DataType, std::size_t,
                                               DataType(const DataType &)>,
                      serial_task.l(), serial_task.r(), serial_task.n(),
                      trap_integral::givenFuncDerivative<const double &>)};

  std::cout << ("Thread Result: " + thread_res.toString() + "\n");
  std::cout << ("Serial Result: " + serial_res.toString() + "\n");

  std::cout << "Verify Thread Result: ==============================\n";
  std::cout << "Same: " << std::boolalpha
            << (decltype(thread_res)::sameResult(thread_res, serial_res))
            << "\n";

  std::cout << ("Performance: ==============================\n");
  std::cout << (p.toString() + "\n");
}

namespace trap_integral {
TrapIntegralDataTypeImp threadImp(int num_threads,
                                  const TrapIntegralTask<double> &task) {
  using DataType = TrapIntegralDataTypeImp;
  using TaskType = TrapIntegralTaskImp;
  using ResultType = TrapIntegralResultImp;

  std::vector<TaskType> tasks(num_threads);
  std::vector<DataType> results(num_threads);

  auto f = [&](int my_rank) {
    results[my_rank] =
        serialImp(tasks[my_rank].l(), tasks[my_rank].r(), tasks[my_rank].n(),
                  [](DataType x) { return givenFuncDerivative(x); });
  };

  std::vector<std::thread> threads(num_threads - 1);

  auto h = (task.r() - task.l()) / task.n();
  for (int i = 0; i < num_threads; ++i) {
    std::size_t l = 0;
    std::size_t r = 0;
    distributeTask(i, num_threads, task.n(), &l, &r);
    tasks[i] = TaskType{task.l() + l * h, task.l() + r * h, r - l};
  }

  for (int i = 1; i < num_threads; ++i) {
    threads[i - 1] = std::thread(f, i);
  }

  for (int i = 1; i < num_threads; ++i) {
    threads[i - 1].join();
  }

  f(0);

  return std::accumulate(results.begin(), results.end(), 0.0);
}
}  // namespace trap_integral
