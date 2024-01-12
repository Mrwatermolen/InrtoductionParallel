#include <thread>
#include <vector>

#include "helper.h"
#include "homework.h"

int main(int argc, char **argv) {
  auto num_threads = getArgNumThread(argc, argv);
  if (num_threads == 0) {
    return 1;
  }

  if (std::thread::hardware_concurrency() < num_threads) {
    std::cout << "Max threads is " << std::thread::hardware_concurrency()
              << "\n";
    num_threads = std::thread::hardware_concurrency();
  }

  std::cout << threadConfigureToString(num_threads) << "\n";

  auto task = bin::BinTask<double, std::vector<double>, std::size_t,
                           std::vector<std::size_t>>::createFromInput();
  auto serial_task = task;

  auto thread_time = measureTime(bin::threadImp, num_threads, std::ref(task));
  std::cout << "Thread Result: ==============================\n";
  std::cout << "Thread Elapsed Time: "
            << std::chrono::duration_cast<std::chrono::milliseconds>(
                   thread_time)
                   .count()
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
                   std::chrono::duration<double>(thread_time));
  std::cout << "Speed up: " << s << "\n";
  auto e = efficiency(s, num_threads);
  std::cout << "Efficiency: " << e << "\n";
}

namespace bin {
void threadImp(int num_threads,
               BinTask<double, std::vector<double>, std::size_t,
                       std::vector<std::size_t>> &task) {
  using TaskType = BinTask<double, std::vector<double>, std::size_t,
                           std::vector<std::size_t>>;

  std::vector<std::vector<std::size_t>> bin_count(num_threads);
  for (auto &v : bin_count) {
    v.resize(task.binCount().size());
    for (auto &&i : v) {
      i = 0;
    }
  }

  auto f = [&](int my_rank, std::size_t l, std::size_t r) {
    serialImp(task.data(), l, r, bin_count[my_rank], task.binMaxes(),
              task.info().binN(), task.info().binMin());
  };

  std::vector<std::thread> threads(num_threads - 1);
  std::size_t l;
  std::size_t r;
  for (int i = 1; i < num_threads; ++i) {
    distributeTask(i, num_threads, task.info().n(), &l, &r);
    threads[i - 1] = std::thread(f, i, l, r);
  }
  distributeTask(0, num_threads, task.info().n(), &l, &r);

  f(0, l, r);

  for (auto &t : threads) {
    t.join();
  }

  for (std::size_t i = 0; i < task.binCount().size(); ++i) {
    for (auto &j : bin_count) {
      task.binCount()[i] += j[i];
    }
  }
}
}  // namespace bin
