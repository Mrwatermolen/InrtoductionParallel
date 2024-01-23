#include <gtest/gtest.h>

#include <list>
#include <random>
#include <sstream>

#include "exercises/thread_safe_structure/thread_pool.h"

auto listToString(const std::list<int>& list) {
  std::stringstream ss;
  for (const auto& item : list) {
    ss << item << " ";
  }
  return ss.str();
}

struct SortList {
  exercises::t_s_s::ThreadPool& _pool;

  std::list<int> operator()(std::list<int> list) {
    if (list.empty()) {
      return list;
    }

    auto res = std::list<int>();
    res.splice(res.begin(), list, list.begin());
    const auto& pivot = *res.begin();
    auto divide_point =
        std::partition(list.begin(), list.end(),
                       [&](const int& item) { return item < pivot; });
    auto new_list_low = std::list<int>();
    new_list_low.splice(new_list_low.end(), list, list.begin(), divide_point);
    auto list_low_future =
        _pool.submit(SortList{_pool}, std::move(new_list_low));
    auto new_list_high = SortList{_pool}(std::move(list));

    while (list_low_future.wait_for(std::chrono::seconds(0)) !=
           std::future_status::ready) {
      _pool.runPendingTask();
    }

    res.splice(res.end(), new_list_high);
    res.splice(res.begin(), list_low_future.get());

    return res;
  }
};

TEST(ThreadPool, SimpleQuickSort) {
  using exercises::t_s_s::ThreadPool;

  auto seed = std::chrono::system_clock::now().time_since_epoch().count();
  std::default_random_engine generator(seed);
  std::uniform_int_distribution<int> distribution(1, 100);

  constexpr int n = 30;
  std::list<int> random_list;
  for (int i = 0; i < n; ++i) {
    random_list.emplace_back(distribution(generator));
  }
  auto serial_list = std::list<int>(random_list);

  std::cout << "Before sorting: " << listToString(random_list) << '\n';

  ThreadPool pool(4);

  auto res = pool.submit(SortList{pool}, std::move(random_list));
  while (res.wait_for(std::chrono::seconds(0)) != std::future_status::ready) {
    pool.runPendingTask();
  }

  serial_list.sort();

  auto sorted = res.get();

  std::cout << "After sorting: " << listToString(sorted) << '\n';

  EXPECT_EQ((serial_list == sorted), true);
}


TEST(ThreadPool, LargeQuickSort) {
  using exercises::t_s_s::ThreadPool;

  auto seed = std::chrono::system_clock::now().time_since_epoch().count();
  std::default_random_engine generator(seed);
  std::uniform_int_distribution<int> distribution(1, 100);

  constexpr int n = 8000;
  std::list<int> random_list;
  for (int i = 0; i < n; ++i) {
    random_list.emplace_back(distribution(generator));
  }
  auto serial_list = std::list<int>(random_list);

  // std::cout << "Before sorting: " << listToString(random_list) << '\n';

  ThreadPool pool;

  auto res = pool.submit(SortList{pool}, std::move(random_list));
  while (res.wait_for(std::chrono::seconds(0)) != std::future_status::ready) {
    pool.runPendingTask();
  }

  serial_list.sort();

  auto sorted = res.get();

  // std::cout << "After sorting: " << listToString(sorted) << '\n';

  EXPECT_EQ((serial_list == sorted), true);
}
