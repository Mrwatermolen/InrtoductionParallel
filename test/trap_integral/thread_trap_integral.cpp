#include <numeric>
#include <thread>
#include <vector>

#include "helper.h"
#include "homework/trap_integral.h"

int main(int argc, char **argv) {
  auto num_threads = getArgNumThread(argc, argv);
  if (num_threads == 0) {
    return 0;
  }

  std::cout << threadConfigureToString(num_threads) << "\n";

  using DataType = homework::trap_integral::TrapIntegralDataTypeImp;
  using TaskType = homework::trap_integral::TrapIntegralTaskImp;
  using ResultType = homework::trap_integral::TrapIntegralResultImp;

  auto input_task = TaskType::createFromInput();
  auto thread_task = input_task;
  auto serial_task = input_task;

  std::cout << "Task Info: ==============================\n";
  std::cout << input_task.toString() << "\n";

  auto p = PerformanceCompare{
      num_threads, homework::trap_integral::threadImp,
      homework::trap_integral::serialImp<DataType, std::size_t,
                                         DataType(const DataType &)>};
  auto thread_res =
      ResultType{p.executeParallel(num_threads, std::ref(thread_task))};
  auto serial_res = ResultType{p.executeSerial(
      serial_task.l(), serial_task.r(), serial_task.n(),
      homework::trap_integral::givenFuncDerivative<const double &>)};

  std::cout << ("Result: ==============================\n");
  std::cout << ("Thread Result: " + thread_res.toString() + "\n");
  std::cout << ("Serial Result: " + serial_res.toString() + "\n");

  std::cout << "Verify Thread Result: ==============================\n";
  std::cout << "Same: " << std::boolalpha
            << (decltype(thread_res)::sameResult(thread_res, serial_res))
            << "\n";

  std::cout << ("Performance: ==============================\n");
  std::cout << (p.toString() + "\n");
}
