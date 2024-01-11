#ifndef __PROJECT_NAME_HOMEWORK_H__
#define __PROJECT_NAME_HOMEWORK_H__

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
                    const T& min_meas) {
  if (data < min_meas || bin_maxes[bin_n - 1] <= data) {
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
inline auto serialImp(const TArray& data, SizeType n, SizeTypeArray&& bin_count,
                      const TArray& bin_maxes, SizeType bin_n,
                      const T& min_meas) {
  for (SizeType i = 0; i < n; ++i) {
    auto bin = findBin(data[i], bin_maxes, bin_n, min_meas);
    if (bin == static_cast<SizeType>(-1)) {
      continue;
    }
    bin_count[bin] += 1;
  }
}

template <typename T, typename TArray, typename SizeType,
          typename SizeTypeArray>
struct BinTask {
  SizeType _n;
  T _min;
  T _max;
  SizeType _bin_n;
  T _bin_min;
  T _bin_max;

  TArray _data;
  TArray _bin_maxes;
  SizeTypeArray _bin_count;

  std::string headDataToString() {
    std::stringstream ss;
    ss << "Data: ";
    int i = 0;
    while (true) {
      if (_n <= i) {
        break;
      }

      if (i == 10) {
        if (10 < _n) {
          ss << "... ";
        }
        break;
      }

      ss << _data[i] << " ";
      ++i;
    }
    return ss.str();
  }

  std::string taskToString() {
    std::stringstream ss;
    ss << "Task:\n";
    ss << "n: " << _n << "\n";
    ss << "min: " << _min << "\n";
    ss << "max: " << _max << "\n";
    ss << "bin_n: " << _bin_n << "\n";
    ss << "bin_min: " << _bin_min << "\n";
    ss << "bin_max: " << _bin_max << "\n";
    ss << headDataToString();
    return ss.str();
  }

  std::string resToString() {
    std::stringstream ss;
    for (SizeType i = 0; i < _bin_n; ++i) {
      ss << "Bin #" << i << " ["
         << _bin_maxes[i] - (_bin_max - _bin_min) / _bin_n << ", "
         << _bin_maxes[i] << "): " << _bin_count[i];
      if (i != _bin_n - 1) {
        ss << "\n";
      }
    }
    return ss.str();
  }

  auto n() { return _n; }

  auto min() { return _min; }

  auto max() { return _max; }

  auto binN() { return _bin_n; }

  auto binMin() { return _bin_min; }

  auto binMax() { return _bin_max; }

  auto& data() { return _data; }

  auto& binMaxes() { return _bin_maxes; }

  auto& binCount() { return _bin_count; }

  auto resetCount() {
    for (SizeType i = 0; i < _bin_n; ++i) {
      _bin_count[i] = 0;
    }
  }

  auto generateRandomData() {
    static std::default_random_engine gen;
    static std::uniform_real_distribution<T> dis(_min, _max);
    for (SizeType i = 0; i < _n; ++i) {
      _data[i] = dis(gen);
    }
  }

  auto split(int num_thread) {
    std::vector<BinTask> tasks;
    for (int i = 0; i < num_thread; ++i) {
      SizeType start = 0;
      SizeType end = 0;
      distributeTask(i, num_thread, _n, &start, &end);
      auto new_data = TArray(end - start);
      for (SizeType j = start; j < end; ++j) {
        new_data[j - start] = _data[j];
      }
      tasks.emplace_back(end - start, _min, _max, _bin_n, _bin_min, _bin_max,
                         new_data, _bin_maxes, _bin_count);
      tasks.back().resetCount();
    }

    return tasks;
  }

  static auto createFromInput() {
    BinTask task;
    SizeType n = 0;
    T min = 0;
    T max = 0;
    SizeType bin_n = 0;
    T bin_min = 0;
    T bin_max = 0;

    std::cout << "Input n, min, max, bin_n, bin_min, bin_max:\n";
    std::cin >> n >> min >> max >> bin_n >> bin_min >> bin_max;

    task._n = n;
    task._min = min;
    task._max = max;
    task._bin_n = bin_n;
    task._bin_min = bin_min;
    task._bin_max = bin_max;

    task._data = TArray(n);
    task._bin_maxes = TArray(bin_n);
    task._bin_count = SizeTypeArray(bin_n);

    task.generateRandomData();

    auto bin_width = (bin_max - bin_min) / bin_n;
    for (SizeType i = 0; i < bin_n; ++i) {
      task._bin_maxes[i] = bin_min + bin_width * (i + 1);
    }
    for (std::size_t i = 0; i < bin_n; ++i) {
      task._bin_count[i] = 0;
    }

    return task;
  }
};

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

#endif  // __PROJECT_NAME_HOMEWORK_H__
