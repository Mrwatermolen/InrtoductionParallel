#include <omp.h>

#include <functional>
#include <iostream>

#include "helper.h"
#include "homework.h"

int main(int argc, char **argv) {
  auto num_threads = getArgNumThread(argc, argv);
  if (num_threads == 0) {
    return 1;
  }
  if (omp_get_max_threads() < num_threads) {
    std::cout << "Max threads is " << omp_get_max_threads() << "\n";
    num_threads = omp_get_max_threads();
  }

  std::cout << ompConfigureToString(num_threads) << "\n";

  // omp for example
  // int i = 0;

  auto task = bin::BinTask<double, std::vector<double>, std::size_t,
                           std::vector<std::size_t>>::createFromInput();
  auto serial_task = task;

  auto omp_time = measureTime(bin::ompImp, num_threads, std::ref(task));
  std::cout << "Omp Result: ==============================\n";
  std::cout
      << "Omp Elapsed Time: "
      << std::chrono::duration_cast<std::chrono::milliseconds>(omp_time).count()
      << " ms\n";
  std::cout << task.info().toString() << "\n";
  std::cout << task.headDataToString() << "\n";
  std::cout << task.resToString() << "\n";

  auto serial_time = measureTime([&]() {
    bin::serialImp(serial_task.data(), 0UL, serial_task.info().n(),
                   serial_task.binCount(), serial_task.binMaxes(),
                   serial_task.info().binN(), serial_task.info().binMin());
  });
  std::cout << "Serial Result: ==============================\n";
  std::cout << "Serial Elapsed Time: "
            << std::chrono::duration_cast<std::chrono::milliseconds>(
                   serial_time)
                   .count()
            << " ms\n";
  std::cout << serial_task.info().toString() << "\n";
  std::cout << serial_task.headDataToString() << "\n";
  std::cout << serial_task.resToString() << "\n";

  // compare
  std::cout << "Compare Result: ==============================\n";
  std::cout << "Is same result: "
            << bin::sameResult(serial_task.binCount(), task.binCount(),
                               serial_task.info().binN())
            << "\n";
  auto s = speedUp(std::chrono::duration<double>(serial_time),
                   std::chrono::duration<double>(omp_time));
  std::cout << "Speed up: " << s << "\n";
  auto e = efficiency(s, num_threads);
  std::cout << "Efficiency: " << e << "\n";
}

namespace bin {

void ompImp(int num_threads, BinTask<double, std::vector<double>, std::size_t,
                                     std::vector<std::size_t>> &task) {
  const auto end = task.info().n();

  auto &&bin_count = task.binCount();
#pragma omp parallel for num_threads(num_threads)
  for (std::size_t i = 0; i < end; ++i) {
    const auto &data = task.data();
    const auto &bin_maxes = task.binMaxes();
    const auto &bin_min = task.info().binMin();
    const auto &bin_n = task.info().binN();

    auto bin_index = bin::findBin(data[i], bin_maxes, bin_n, bin_min);
    if (bin_index == -1) {
      continue;
    }
    ++bin_count[bin_index];
  }

} // namespace bin

} // namespace bin
