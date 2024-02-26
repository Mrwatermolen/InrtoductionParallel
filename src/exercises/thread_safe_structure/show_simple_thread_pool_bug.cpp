#include <cstdlib>
#include <iostream>
#include <list>
#include <random>
#include <sstream>

#include "exercises/thread_safe_structure/simple_thread_pool.h"

// simple thread pool can't handle the task that waits other tasks to finish.
// Result: anyone is waiting for the low list to finish, no one to take the task
// from the queue

int main() {
  using exercises::t_s_s::bug_version::ThreadPool;

  std::list<int> radom_list;
  int n = 10;
  auto seed = std::chrono::system_clock::now().time_since_epoch().count();
  std::default_random_engine generator(seed);
  std::uniform_int_distribution<int> distribution(1, 100);
  for (int i = 0; i < n; ++i) {
    radom_list.emplace_back(distribution(generator));
  }

  auto print_list = [](const std::list<int>& list) {
    for (const auto& item : list) {
      std::cout << item << " ";
    }
    std::cout << '\n';
  };

  std::cout << "Before sorting: ";
  print_list(radom_list);

  ThreadPool pool(2);

  struct SortList {
    ThreadPool& _pool;

    std::list<int> operator()(std::list<int> list) {
      if (list.empty()) {
        return list;
      }

      std::stringstream ss;
      ss << "Thread: " << std::this_thread::get_id() << " sorting ";
      for (const auto& item : list) {
        ss << item << " ";
      }
      ss << "\n";

      auto res = std::list<int>();
      res.splice(res.begin(), list, list.begin());
      const auto& pivot = *res.begin();
      auto divide_point =
          std::partition(list.begin(), list.end(),
                         [&](const int& item) { return item < pivot; });
      auto new_list_low = std::list<int>();
      new_list_low.splice(new_list_low.end(), list, list.begin(), divide_point);

      ss << "Waiting for low list to finish: ";
      for (const auto& item : new_list_low) {
        ss << item << " ";
      }
      ss << "\n";
      std::cout << ss.str();

      auto new_list_low_future =
          _pool.submit(SortList{_pool}, std::move(new_list_low));
      auto new_list_high = SortList{_pool}(std::move(list));
      res.splice(res.end(), new_list_high);
      auto l = new_list_low_future.get();
      res.splice(res.begin(), l);
      return res;
    }
  };

  auto sorted_list = pool.submit(SortList{pool}, std::move(radom_list));
  if (sorted_list.wait_for(std::chrono::seconds(10)) ==
      std::future_status::timeout) {
    std::cout << "Timeout\n";
    exit(1);
  }
  auto l = sorted_list.get();
  std::cout << "After sorting: ";
  print_list(l);
}
