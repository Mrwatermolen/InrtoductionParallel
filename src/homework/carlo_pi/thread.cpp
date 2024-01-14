#include <cstddef>
#include <thread>

#include "helper.h"
#include "homework/carlo_pi.h"

namespace homework::carlo_pi {

std::size_t threadImp(int num_threads, const TaskTypeImp &task) {
  using TaskType = TaskTypeImp;

  std::vector<std::thread> threads(num_threads - 1);
  std::vector<std::size_t> counters(num_threads, 0);

  auto f = [&](int my_rank) {
    std::size_t l;
    std::size_t r;
    distributeTask(my_rank, num_threads, task.n(), &l, &r);
    counters[my_rank] = serialImp(r - l, task.radius());
  };

  for (int i = 1; i < num_threads; ++i) {
    threads[i - 1] = std::thread(f, i);
  }

  f(0);

  for (auto &t : threads) {
    t.join();
  }

  return std::accumulate(counters.begin(), counters.end(),
                         static_cast<std::size_t>(0));
}

}  // namespace homework::carlo_pi
