#include <thread>
#include <vector>

#include "helper.h"
#include "homework.h"

int main(int argc, char **argv) {}

namespace trap_integral {
// TrapIntegralResultImp threadImp(int num_threads,
//                                 TrapIntegralTask<double> &task) {
//   using DataType = TrapIntegralDataTypeImp;
//   using TaskType = TrapIntegralTaskImp;
//   using ResultType = TrapIntegralResultImp;

//   std::vector<TaskType> tasks(num_threads);

//   auto f = [&](int my_rank) {
//     return serialImp(tasks[my_rank].l(), tasks[my_rank].r(), tasks[my_rank].n(),
//                      [](DataType x) { return givenFuncDerivative(x); });
//   };

//   std::vector<std::thread> threads(num_threads - 1);
//   // std::vector<std::packaged_task<decltype(f)>> packaged_tasks(num_threads - 1);

//   auto h = (task.r() - task.l()) / task.n();
//   for (int i = 0; i < num_threads; ++i) {
//     std::size_t l = 0;
//     std::size_t r = 0;
//     distributeTask(i, num_threads, task.n(), &l, &r);
//     tasks[i] = TaskType{task.l() + l * h, task.l() + r * h, r - l};
//   }

//   for (int i = 1; i < num_threads; ++i) {
//     threads[i - 1] = std::thread(f, i);
//   }

//   for (int i = 1; i < num_threads; ++i) {
//     threads[i - 1].join();
//   }

//   auto sum = f(0);

//   for (int i = 1; i < num_threads; ++i) {
//     // sum += packaged_tasks[i - 1].get_future().get();
//   }

//   return {};
// }
}  // namespace trap_integral
