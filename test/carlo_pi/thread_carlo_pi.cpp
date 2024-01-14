#include "helper.h"
#include "homework/carlo_pi.h"

int main(int argc, char **argv) {
  auto num_threads = getArgNumThread(argc, argv);

  using TaskType = homework::carlo_pi::TaskTypeImp;
  using DataType = homework::carlo_pi::DataTypeImp;
  using ResultType = homework::carlo_pi::ResultTypeImp;

  TaskType input_task;
  TaskType thread_task;
  TaskType serial_task;

  std::cout << threadConfigureToString(num_threads) << "\n";
  input_task = TaskType::createFromInput();
  thread_task = input_task;
  serial_task = input_task;

  std::cout << "Task info: ======================\n";
  std::cout << input_task.toString() << "\n";

  std::cout << "Running\n";

  auto pc = PerformanceCompare{num_threads, homework::carlo_pi::threadImp,
                               homework::carlo_pi::serialImp<DataType>};

  auto thread_res = ResultType{
      pc.executeParallel(num_threads, std::ref(thread_task)), thread_task.n()};

  auto serial_res = ResultType{
      pc.executeSerial(serial_task.n(), serial_task.radius()), serial_task.n()};

  std::cout << "Thread Result: ======================\n";
  std::cout << thread_res.toString() << "\n";

  std::cout << "Serial Result: ======================\n";
  std::cout << serial_res.toString() << "\n";

  std::cout << "Verify Result: ======================\n";
  std::cout << ("Same: ") << std::boolalpha
            << (ResultType::sameResult(serial_res, thread_res)) << "\n";

  std::cout << "Performance Compare: ==============================\n";
  std::cout << pc.toString() << "\n";

  return 0;
}
