#ifndef __PROJECT_NAME_HOMEWORK_H__
#define __PROJECT_NAME_HOMEWORK_H__

#include <chrono>
#include <cmath>
#include <cstddef>
#include <iostream>
#include <random>
#include <sstream>
#include <vector>

#include "helper.h"

namespace global_sum {

void mpiRun();

double computeNextValueWithRandomSleep();

double computeNextValue();

template <typename T>
std::string taskToString(T l, T r);

void domSum(int my_rank, int size, double* sum);

void mpiPlatSum(int my_rank, int size, int tag, double* sum);

void mpiTreeSum(int my_rank, int size, int tag, double* sum);

void mpiTreeSum01(int my_rank, int size, int divisor, int core_different,
                  double* sum, int tag);

template <typename T>
inline auto serialSum(T&& l, T&& r, bool sleep = true) {
  double sum = 0;
  if (sleep) {
    for (int i = l; i < r; ++i) {
      sum += computeNextValueWithRandomSleep();
    }
    return sum;
  }

  for (int i = l; i < r; ++i) {
    sum += computeNextValue();
  }
  return sum;
}

}  // namespace global_sum

namespace bin {

template <typename T, typename TArray, typename SizeType>
inline auto findBin(const T& data, const TArray& bin_maxes, SizeType bin_n,
                    const T& bin_min) {
  if (data < bin_min || bin_maxes[bin_n - 1] <= data) {
    return static_cast<SizeType>(-1);
  }

  auto binary_find = [](auto self, const T& data, const TArray& bin_maxes,
                        SizeType start, SizeType end) {
    if (end <= start) {
      return start;
    }
    SizeType mid = (start + end) / 2;
    if (data < bin_maxes[mid]) {
      return self(self, data, bin_maxes, start, mid);
    }

    return self(self, data, bin_maxes, mid + 1, end);
  };

  return binary_find(binary_find, data, bin_maxes, 0, bin_n);
}

template <typename T, typename TArray, typename SizeType = std::size_t,
          typename SizeTypeArray>
inline auto serialImp(const TArray& data, SizeType l, SizeType r,
                      SizeTypeArray&& bin_count, const TArray& bin_maxes,
                      SizeType bin_n, const T& bin_min) {
  for (SizeType i = l; i < r; ++i) {
    auto bin = findBin(data[i], bin_maxes, bin_n, bin_min);
    if (bin == static_cast<SizeType>(-1)) {
      continue;
    }
    bin_count[bin] += 1;
  }
}

template <typename T, typename TArray, typename SizeType,
          typename SizeTypeArray>
struct BinTask {
  struct BinTaskInfo {
    SizeType _n{};
    T _min{};
    T _max{};
    SizeType _bin_n{};
    T _bin_min{};
    T _bin_max{};

    auto& n() { return _n; }

    auto& min() { return _min; }

    auto& max() { return _max; }

    auto& binN() { return _bin_n; }

    auto& binMin() { return _bin_min; }

    auto& binMax() { return _bin_max; }

    std::string toString() const {
      std::stringstream ss;
      ss << "Task:\n";
      ss << "n: " << _n << "\n";
      ss << "min: " << _min << "\n";
      ss << "max: " << _max << "\n";
      ss << "bin_n: " << _bin_n << "\n";
      ss << "bin_min: " << _bin_min << "\n";
      ss << "bin_max: " << _bin_max;
      return ss.str();
    }

    static auto createFromInput() {
      SizeType n = 0;
      T min = 0;
      T max = 0;
      SizeType bin_n = 0;
      T bin_min = 0;
      T bin_max = 0;

      std::cout << "Input n, min, max, bin_n, bin_min, bin_max:\n";
      std::cin >> n >> min >> max >> bin_n >> bin_min >> bin_max;

      return BinTaskInfo{n, min, max, bin_n, bin_min, bin_max};
    }
  };

  BinTaskInfo _info{};

  TArray _bin_maxes{};

  TArray _data{};
  SizeTypeArray _bin_count{};

  std::string headDataToString() {
    std::stringstream ss;
    ss << "Data: ";
    int i = 0;
    auto&& n = _info.n();
    while (true) {
      if (n <= i) {
        break;
      }

      if (i == 10) {
        if (10 < n) {
          ss << "... ";
        }
        break;
      }

      ss << _data[i] << " ";
      ++i;
    }
    return ss.str();
  }

  std::string resToString() {
    std::stringstream ss;
    SizeType sum = 0;
    for (SizeType i = 0; i < _info.binN(); ++i) {
      sum += _bin_count[i];
      ss << "Bin #" << i << " ["
         << _bin_maxes[i] - (_info.binMax() - _info.binMin()) / _info.binN()
         << ", " << _bin_maxes[i] << "): " << _bin_count[i] << "\n";
    }
    ss << "Total number in bin: " << sum;
    return ss.str();
  }

  auto& info() { return _info; }

  auto& data() { return _data; }

  auto& binMaxes() { return _bin_maxes; }

  auto& binCount() { return _bin_count; }

  auto resetCount() {
    // bug here!!! QAQ
    for (auto&& c : _bin_count) {
      c = 0;
    }
  }

  auto generateRandomData() {
    static std::default_random_engine gen;
    static std::uniform_real_distribution<T> dis(_info.min(), _info.max());
    for (SizeType i = 0; i < _info.n(); ++i) {
      _data[i] = dis(gen);
    }
  }

  auto split(int num_thread) {
    std::vector<BinTask> tasks;
    for (int i = 0; i < num_thread; ++i) {
      SizeType start = 0;
      SizeType end = 0;
      distributeTask(i, num_thread, _info.n(), &start, &end);
      auto new_data = TArray(end - start);
      for (SizeType j = start; j < end; ++j) {
        new_data[j - start] = _data[j];
      }
      tasks.emplace_back(
          BinTaskInfo{end - start, _info.min(), _info.max(), _info.binN(),
                      _info.binMin(), _info.binMax()},
          new_data, _bin_maxes, _bin_count);
      tasks.back().resetCount();
    }

    return tasks;
  }

  auto generateDataFromInfo(BinTaskInfo info) {
    _info = info;

    auto n = _info.n();
    auto bin_n = _info.binN();

    _data = TArray(n);
    generateRandomData();
    _bin_maxes = binMaxesFromInfo(_info);
    _bin_count = SizeTypeArray(bin_n);
    resetCount();
  }

  static auto createFromInput() {
    BinTask task;
    task.generateDataFromInfo(BinTaskInfo::createFromInput());
    return task;
  }

  static auto binMaxesFromInfo(BinTaskInfo& info) {
    auto bin_n = info.binN();
    auto bin_min = info.binMin();
    auto bin_max = info.binMax();

    auto bin_width = (bin_max - bin_min) / bin_n;
    auto bin_maxes = TArray(bin_n);
    for (SizeType i = 0; i < bin_n; ++i) {
      bin_maxes[i] = bin_min + bin_width * (i + 1);
    }
    return bin_maxes;
  }
};

void mpiImp(int my_rank, int size,
            BinTask<double, std::vector<double>, std::size_t,
                    std::vector<std::size_t>>& task,
            bool printDataCopyTime = false);

void threadImp(int num_threads,
               BinTask<double, std::vector<double>, std::size_t,
                       std::vector<std::size_t>>& task);

void ompImp(int num_threads, BinTask<double, std::vector<double>, std::size_t,
                                     std::vector<std::size_t>>& task);

template <typename SizeType, typename SizeTypeArray>
inline auto sameResult(const SizeTypeArray& l, const SizeTypeArray& r,
                       SizeType bin_n) {
  for (SizeType i = 0; i < bin_n; ++i) {
    if (l[i] != r[i]) {
      return false;
    }
  }
  return true;
}

}  // namespace bin

namespace trap_integral {

template <typename T>
struct TrapIntegralTask {
  T _l{};
  T _r{};
  std::size_t _n{};

  auto& l() { return _l; }

  auto& r() { return _r; }

  auto& n() { return _n; }

  const auto& l() const { return _l; }

  const auto& r() const { return _r; }

  const auto& n() const { return _n; }

  std::string toString() const {
    std::stringstream ss;
    ss << "Task: "
       << "sample points: " << _n << " domain: [" << _l << ", " << _r << ")";
    return ss.str();
  }

  std::size_t bytes() const { return sizeof(*this); }

  static auto createFromInput() {
    T l{};
    T r{};

    auto n = getInoutOneDimensionProblemSize();
    std::cout << "Input l, r:\n";
    std::cin >> l >> r;

    return TrapIntegralTask{l, r, n};
  }
};

template <typename TimeDuration, typename T>
struct TrapIntegralResult {
  T _res{};

  auto& res() { return _res; }

  const auto& res() const { return _res; }

  std::string toString(int precision = 6) const {
    std::stringstream ss;
    ss.precision(precision);
    ss << std::fixed;

    ss << "Value: " << _res;

    return ss.str();
  }

  inline static T epsilon = 1e-6;
  static auto sameResult(const TrapIntegralResult& l,
                         const TrapIntegralResult& r) {
    return l._res == r._res || std::abs(l._res - r._res) < epsilon;
  }
};

template <typename T>
inline auto givenFunc(const T& x)
    -> decltype(std::sin(x) * std::cos(x) * std::exp(-x)) {
  return std::sin(x) * std::cos(x) * std::exp(-x);
}

template <typename T>
inline auto givenFuncDerivative(const T& x)
    -> decltype(-std::exp(-x) * (0.5 * std::sin(2 * x) + std::cos(2 * x))) {
  return std::exp(-x) * (-0.5 * std::sin(2 * x) + std::cos(2 * x));
}

template <typename T, typename SizeType, typename Func>
inline auto serialImp(const T& l, const T& r, SizeType n, Func&& f) {
  auto h = (r - l) / n;
  auto sum = (f(l) + f(r)) / 2;
  for (int i = 1; i < n; ++i) {
    sum += f(l + i * h);
  }
  return sum * h;
}

using TrapIntegralDataTypeImp = double;
using TrapIntegralTaskImp = TrapIntegralTask<TrapIntegralDataTypeImp>;
using TrapIntegralResultImp =
    TrapIntegralResult<std::chrono::high_resolution_clock::duration,
                       TrapIntegralDataTypeImp>;

TrapIntegralDataTypeImp mpiImp(int my_rank, int size, TrapIntegralTaskImp& task,
                             bool printDataCopyTime = false);

TrapIntegralDataTypeImp threadImp(int num_threads, const TrapIntegralTaskImp& task);

TrapIntegralDataTypeImp ompImp(int num_threads, const TrapIntegralTaskImp& task);

}  // namespace trap_integral

#endif  // __PROJECT_NAME_HOMEWORK_H__
