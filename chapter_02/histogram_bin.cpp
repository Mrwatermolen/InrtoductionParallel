#include <cstddef>
#include <iostream>
#include <vector>

#include "helper.h"
#include "homework.h"

int main(int argc, char** argv) {
  int num_threads = getArgNumThread(argc, argv);
  if (num_threads == 0) {
    return 1;
  }

  auto&& task = bin::BinTask<double, std::vector<double>, std::size_t,
                             std::vector<std::size_t>>::createFromInput();

  std::cout << task.info().toString() << "\n";

  auto f = [&]() {
    bin::serialImp(task.data().data(), 0UL, task.info().n(),
                   task.binCount().data(), task.binMaxes().data(),
                   task.info().binN(), task.info().binMin());
  };

  // auto serial_time =
  // measureTime(bin::serialImp, data, n, bin_count, bin_maxes, bin_n,
  // bin_min);  // error

  auto serial_time = measureTime(f);
  std::cout << "Serial Result: ==============================\n";
  std::cout << "Serial Elapsed Time: "
            << std::chrono::duration_cast<std::chrono::milliseconds>(
                   serial_time)
                   .count()
            << " ms\n";

  std::cout << task.resToString() << "\n";

  auto&& sub_tasks = task.split(num_threads);

  return 0;
}