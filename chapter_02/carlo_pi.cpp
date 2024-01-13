#include "helper.h"
#include "homework.h"

#include <cmath>

int main() {
  auto task = carlo_pi::CarloPITask::createFromInput();

  std::cout << "Task Info: ==============================\n";
  std::cout << task.toString() << "\n";

  auto p = ExecutionProfile{};
  auto serial_res = carlo_pi::CarloPIResult{
      p.execute(carlo_pi::serialImp, task.n(), task.radius()), task.n()};

  std::cout << ("Result: ==============================\n");
  std::cout << "Serial Result: " << serial_res.toString(4) << "\n";

  std::cout << "Exact Result: " << M_PI << "\n";

  std::cout << ("Verify Serial Result: ==============================\n");
  carlo_pi::CarloPIResult::epsilon = 1e-3;
  std::cout << "Same: " << std::boolalpha
            << (decltype(serial_res)::sameResult(serial_res, carlo_pi::CarloPIResult{M_PI})) << "\n";
  
  std::cout << ("Performance: ==============================\n");
  std::cout << p.toString() << "\n";
}
