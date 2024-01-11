#ifndef __PROJECT_NAME_HELPER_H__
#define __PROJECT_NAME_HELPER_H__

#include <chrono>
#include <iostream>
#include <utility>

inline int getArgNumThread(int argc, char** argv) {
  if (2 < argc) {
    std::cout << "Usage: " << argv[0] << "<n> \n";
    return 0;
  }
  int num_threads = 1;
  if (argc == 2) {
    num_threads = std::stoi(argv[1]);
    if (num_threads < 1) {
      std::cerr << "num_threads must be greater than 0\n";
      return 0;
    }
  }
  return num_threads;
}

inline auto getInoutOneDimensionProblemSize() {
  std::size_t n = 0;
  std::cout << "Input n:\n";
  std::cin >> n;
  return n;
}

template <typename T>
inline void distributeTask(int my_rank, int size, T n, T* start, T* end) {
  auto quotient = n / size;
  auto remainder = n % size;
  if (my_rank < remainder) {
    *start = my_rank * (quotient + 1);
    *end = *start + quotient + 1;
  } else {
    *start = my_rank * quotient + remainder;
    *end = *start + quotient;
  }
}

template <typename SendFunc, typename RecvFunc>
inline auto treeCommunication(int my_rank, int size, SendFunc&& s,
                              RecvFunc&& r) {
  auto f = [&](auto&& self, auto&& flag) {
    if (size <= flag) {
      return;
    }

    auto partner = my_rank ^ flag;
    if (size <= partner) {
      self(self, (flag << 1));
      return;
    }

    bool right_node = (my_rank & flag) != 0;
    if (right_node) {
      s(partner);
      return;
    }

    r(partner);
    self(self, (flag << 1));
  };

  f(f, 1);
}

template <typename SRFunc, typename RSFunc, typename SendFunc,
          typename RecvFunc>
inline auto platCommunication(int my_rank, int size, SRFunc&& sr, RSFunc&& rs,
                              SendFunc&& s, RecvFunc&& r) {
  auto f = [&](auto&& self, auto&& flag) {
    if (size <= flag) {
      return;
    }

    auto sub_domain_size = (flag << 1);
    auto sub_domain_leader = my_rank - (my_rank % sub_domain_size);
    auto partner = my_rank ^ flag;

    if (size <= partner) {
      auto leader_partner = sub_domain_leader ^ flag;
      if (leader_partner < size) {
        r(sub_domain_leader);
      }
      self(self, (flag << 1));
      return;
    }

    bool recv_then_send = (my_rank & flag) == 0;
    if (recv_then_send) {
      rs(partner);
    } else {
      sr(partner);
    }

    if (my_rank == sub_domain_leader && size < my_rank + sub_domain_size) {
      // simplicity: let leader sender msg to waiter
      auto num_sender = (sub_domain_size >> 1);
      auto num_waiter = sub_domain_size - (size % sub_domain_size);
      auto first_waiter = my_rank + (num_sender - num_waiter);

      for (auto i = 0; i < num_waiter; ++i) {
        s(first_waiter + i);
      }
    }

    self(self, (flag << 1));
  };

  f(f, 1);
}

template <typename Func, typename... Args>
inline auto measureTime(Func func, Args&&... args) {
  using FuncReturnType = decltype(func(std::forward<Args>(args)...));
  using TimeDuration = decltype(std::chrono::high_resolution_clock::now() -
                                std::chrono::high_resolution_clock::now());

  const auto start = std::chrono::high_resolution_clock::now();
  if constexpr (std::is_same_v<void, FuncReturnType>) {
    func(std::forward<Args>(args)...);
    auto end = std::chrono::high_resolution_clock::now();
    return (end - start);
  } else {
    FuncReturnType ret = func(std::forward<Args>(args)...);
    auto end = std::chrono::high_resolution_clock::now();
    return std::tuple<TimeDuration, FuncReturnType>{(end - start), ret};
  }
}

template <typename TimeDuration>
inline auto speedUp(TimeDuration serial_time, TimeDuration parallel_time)
    -> decltype(serial_time / parallel_time) {
  return serial_time / parallel_time;
}

template <typename T, typename N>
inline auto efficiency(T speed_up, N num_threads)
    -> decltype(speed_up / num_threads) {
  return speed_up / num_threads;
}

#endif  // __PROJECT_NAME_HELPER_H__
