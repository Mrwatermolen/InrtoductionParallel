#include "helper.h"
#include "homework.h"
#include "homework/histogram_bin.h"
#include <string>
#include <thread>
#include <vector>

namespace homework::histogram_bin {
SizeTypeArrayImp threadImp(int num_threads, const TaskTypeImp &task,
                           const DataTypeArrayImp &data) {
  using TaskType = TaskTypeImp;
  using DataType = DataTypeImp;
  using DataTypeArray = DataTypeArrayImp;
  using SizeType = SizeTypeImp;
  using SizeTypeArray = SizeTypeArrayImp;

  std::vector<std::thread> threads;

  std::vector<SizeTypeArray> bin_count(num_threads,
                                       SizeTypeArray(task.binN(), 0));

  for (SizeType i = 1; i < num_threads; ++i) {
    SizeType l = 0;
    SizeType r = 0;
    distributeTask(i, num_threads, task.n(), &l, &r);
    threads.emplace_back([l, r, &bin_count, &data, &task, i]() {
      auto bin_maxes =
          TaskType::binMaxesFromInfo(task.binN(), task.binMin(), task.binMax());
      serialImp(data, l, r, bin_count[i], bin_maxes, task.binN(),
                task.binMin());
    });
  }

  SizeType l = 0;
  SizeType r = 0;
  distributeTask(0, num_threads, task.n(), &l, &r);
  auto bin_maxes =
      TaskType::binMaxesFromInfo(task.binN(), task.binMin(), task.binMax());
  serialImp(data, l, r, bin_count[0], bin_maxes, task.binN(), task.binMin());

  for (auto &t : threads) {
    t.join();
  }

  auto bin_count_sum = SizeTypeArray(task.binN(), 0);
  for (SizeType i = 0; i < num_threads; ++i) {
    for (SizeType j = 0; j < task.binN(); ++j) {
      bin_count_sum[j] += bin_count[i][j];
    }
  }
  return bin_count_sum;
}
} // namespace homework::histogram_bin
