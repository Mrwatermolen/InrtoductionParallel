#include "helper.h"
#include "homework/carlo_pi.h"

int main(int argc, char **argv) {
  auto num_threads = getArgNumThread(argc, argv);


  using TaskType = homework::carlo_pi::TaskTypeImp;
  using DataType = homework::carlo_pi::DataTypeImp;
  using ResultType = homework::carlo_pi::ResultTypeImp;

  TaskType input_task;
  TaskType omp_task;
  TaskType serial_task;

  std::cout << ompConfigureToString(num_threads) << "\n";
  input_task = TaskType::createFromInput();
  omp_task = input_task;
  serial_task = input_task;

  std::cout << "Task info: ======================\n";
  std::cout << input_task.toString() << "\n";

  std::cout << "Running\n";

  auto pc = PerformanceCompare{num_threads, homework::carlo_pi::ompImp,
                               homework::carlo_pi::serialImp<DataType>};

  auto omp_res = ResultType{
      pc.executeParallel(num_threads, std::ref(omp_task)), omp_task.n()};

  auto serial_res = ResultType{
      pc.executeSerial(serial_task.n(), serial_task.radius()), serial_task.n()};

  std::cout << "OpenMP Result: ======================\n";
  std::cout << omp_res.toString() << "\n";

  std::cout << "Serial Result: ======================\n";
  std::cout << serial_res.toString() << "\n";

  std::cout << "Verify Result: ======================\n";
  std::cout << ("Same: ") << std::boolalpha
            << (ResultType::sameResult(serial_res, omp_res)) << "\n";

  std::cout << "Performance Compare: ==============================\n";
  std::cout << pc.toString() << "\n";

}
