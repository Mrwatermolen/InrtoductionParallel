#include "helper.h"
#include "homework/carlo_pi.h"

#include <cstddef>
#include <thread>

namespace homework::carlo_pi {

std::size_t threadImp(int num_threads, const TaskTypeImp &task) {
  using TaskType = TaskTypeImp;

  std::vector<std::thread> threads;
  std::vector<std::size_t> counters(num_threads, 0);

  for (int i = 1; i < num_threads; ++i) {
    threads.emplace_back([&counters, i, num_threads, &task]() {
      std::size_t l;
      std::size_t r;
      distributeTask(i, num_threads, task.n(), &l, &r);
      counters[i] = serialImp(r - l, task.radius());
    });
  }

  std::size_t l;
  std::size_t r;
  distributeTask(0, num_threads, task.n(), &l, &r);
  counters[0] = serialImp(r - l, task.radius());

  for (auto &t : threads) {
    t.join();
  }

  return std::accumulate(counters.begin(), counters.end(), static_cast<std::size_t>(0));
}

} // namespace homework::carlo_pi
