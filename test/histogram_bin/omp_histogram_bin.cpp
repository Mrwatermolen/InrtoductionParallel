#include <random>

#include "helper.h"
#include "homework/histogram_bin.h"

int main(int argc, char **argv) {
  auto num_threads = getArgNumThread(argc, argv);
  // print
  std::cout << ompConfigureToString(num_threads) << "\n";

  using TaskType = homework::histogram_bin::TaskTypeImp;
  using DataType = homework::histogram_bin::DataTypeImp;
  using DataTypeArray = homework::histogram_bin::DataTypeArrayImp;
  using SizeType = homework::histogram_bin::SizeTypeImp;
  using SizeTypeArray = homework::histogram_bin::SizeTypeArrayImp;
  using ResultType = homework::histogram_bin::ResultTypeImp;

  auto random_data = [](std::size_t n, double min, double max) {
    static std::default_random_engine gen;
    static std::uniform_real_distribution<> dis(min, max);
    std::vector<double> data(n);
    for (std::size_t i = 0; i < n; ++i) {
      data[i] = dis(gen);
    }
    return data;
  };

  TaskType omp_task;
  TaskType serial_task;
  DataTypeArray data;

  auto &&input_task = TaskType::createFromInput();

  std::cout << "Preparing Data: ==============================\n";
  data = random_data(input_task.n(), input_task.min(), input_task.max());

  std::cout << "Task Info: ==============================\n";
  std::cout << input_task.toString() << "\n"
            << homework::histogram_bin::headDataToString(data, input_task.n())
            << "\n";

  omp_task = input_task;
  serial_task = input_task;

  auto pc = PerformanceCompare{
      num_threads, homework::histogram_bin::ompImp,
      homework::histogram_bin::serialImp<DataType, DataTypeArray, SizeType,
                                         SizeTypeArray>};

  auto omp_res = ResultType{
      pc.executeParallel(num_threads, std::ref(omp_task), std::ref(data))};

  auto bin_maxes = TaskType::binMaxesFromInfo(
      serial_task.binN(), serial_task.binMin(), serial_task.binMax());

  auto serial_bin_count = SizeTypeArray(serial_task.binN(), 0);

  pc.executeSerial(std::ref(data), static_cast<SizeType>(0), serial_task.n(),
                   std::ref(serial_bin_count), std::ref(bin_maxes),
                   serial_task.binN(), serial_task.binMin());

  auto serial_res = ResultType{serial_bin_count};

  std::cout << "OMP Result: ==============================\n";
  std::cout << omp_res.toString(omp_task) << "\n";

  std::cout << "Serial Result: ==============================\n";
  std::cout << serial_res.toString(serial_task) << "\n";

  std::cout << "Verify Result: ==============================\n";
  std::cout << "Same: " << std::boolalpha
            << ResultType::sameResult(serial_res, omp_res, serial_task.binN())
            << "\n";

  std::cout << "Performance Compare: ==============================\n";
  std::cout << pc.toString() << "\n";
}
