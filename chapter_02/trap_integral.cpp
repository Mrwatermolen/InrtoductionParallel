#include <functional>
#include <iostream>

#include "helper.h"
#include "homework.h"

int main(int argc, char **argv) {
  using DataType = trap_integral::TrapIntegralDataTypeImp;
  using TaskType = trap_integral::TrapIntegralTaskImp;
  using ResultType = trap_integral::TrapIntegralResultImp;

  auto task = trap_integral::TrapIntegralTask<double>::createFromInput();

  std::cout << task.toString() << "\n";

  // try to test performance of std::function
  auto func = [](double x) { return trap_integral::givenFuncDerivative(x); };
  auto p = ExecutionProfile{};
  auto serial_res = ResultType{
      p.execute(trap_integral::serialImp<double &, std::size_t,
                                         std::function<double(double)>>,
                task.l(), task.r(), task.n(), func)};
  std::cout << "Serial Result:\n";
  std::cout << p.toString() << "\n";
  std::cout << serial_res.toString() << "\n";

  auto f_exact = [&]() {
    return trap_integral::givenFunc(task.r()) -
           trap_integral::givenFunc(task.l());
  };
  auto exact_res = ResultType{f_exact()};
  std::cout << "Exact Result:\n";
  std::cout << exact_res.toString() << "\n";

  std::cout << "Verify Result:\n";
  std::cout << "Same: " << std::boolalpha
            << (decltype(exact_res)::sameResult(serial_res, exact_res)) << "\n";
}
