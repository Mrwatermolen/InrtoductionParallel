#ifndef __PROJECT_NAME_HISTOGRAM_BIN_H__
#define __PROJECT_NAME_HISTOGRAM_BIN_H__

#include "helper.h"
#include "homework.h"
#include <cstddef>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace homework::histogram_bin {

template <typename T, typename TArray, typename SizeType>
class HistogramBinTask : public Task {
public:
  HistogramBinTask() = default;

  HistogramBinTask(SizeType n, T min, T max, SizeType bin_n, T bin_min,
                   T bin_max)
      : Task(n), _min(min), _max(max), _bin_n(bin_n), _bin_min(bin_min),
        _bin_max(bin_max) {}

  ~HistogramBinTask() override = default;

  const auto& max() const { return _max; }

  const auto& min() const { return _min; }

  const auto& binN() const { return _bin_n; }

  const auto& binMax() const { return _bin_max; };

  const auto& binMin() const { return _bin_min; }

  std::string toString() const override {
    std::stringstream ss;
    ss.precision(precision);
    ss << std::fixed;
    ss << "Task:\n";
    ss << "n: " << n() << "\n";
    ss << "min: " << min() << "\n";
    ss << "max: " << max() << "\n";
    ss << "bin_n: " << binN() << "\n";
    ss << "bin_min: " << binMin() << "\n";
    ss << "bin_max: " << binMax();
    return ss.str();
  }

  std::size_t bytes() const override { return sizeof(*this); }

  inline static int precision = 6;

  static auto createFromInput() {
    auto n = getInoutOneDimensionProblemSize();

    T min = 0;
    T max = 0;
    SizeType bin_n = 0;
    T bin_min = 0;
    T bin_max = 0;

    std::cout << "Input min, max, bin_n, bin_min, bin_max:\n";
    std::cin >> min >> max >> bin_n >> bin_min >> bin_max;

    return HistogramBinTask{n, min, max, bin_n, bin_min, bin_max};
  }

  static auto binMaxesFromInfo(const SizeType &bin_n, const T &bin_min,
                               const T &bin_max) {
    auto bin_width = (bin_max - bin_min) / bin_n;
    auto bin_maxes = TArray(bin_n);

    for (SizeType i = 0; i < bin_n; ++i) {
      bin_maxes[i] = bin_min + bin_width * (i + 1);
    }
    return bin_maxes;
  }

private:
  T _min{}, _max;
  SizeType _bin_n{};
  T _bin_min{}, _bin_max{};
};

template <typename SizeType, typename SizeTypeArray>
class HistogramBinResult : public Result<SizeTypeArray> {
public:
  HistogramBinResult() = default;

  explicit HistogramBinResult(SizeTypeArray bin_count)
      : Result<SizeTypeArray>(std::move(bin_count)) {}

  ~HistogramBinResult() override = default;

  auto &binCount() { return this->res(); }

  const auto &binCount() const { return this->res(); }

  template <typename BinTask> std::string toString(const BinTask &t) const {
    std::stringstream ss;
    SizeType sum = 0;
    ss.precision(BinTask::precision);
    ss << std::fixed;
    ss << "Result:\n";
    auto bin_maxes = BinTask::binMaxesFromInfo(t.binN(), t.binMin(), t.binMax());
    for (SizeType i = 0; i < t.binN(); ++i) {
      sum += binCount()[i];
      ss << "Bin #" << i << " ["
         << bin_maxes[i] - (t.binMax() - t.binMin()) / t.binN() << ", "
         << bin_maxes[i] << "): " << binCount()[i] << "\n";
    }
    ss << "Total number in bin: " << sum;
    return ss.str();
  }

  std::string toString() const override {
    std::stringstream ss;
    ss << "Result:\n";
    SizeType sum = 0;
    SizeType i = 0;
    for (auto &&c : this->res()) {
      sum += c;
      ss << "Bin #" << i << ": " << c << "\n";
      ++i;
    }
    ss << "Total number in bin: " << sum;
    return ss.str();
  }

  static bool sameResult(const HistogramBinResult &l,
                         const HistogramBinResult &r, SizeType bin_n) {

    for (SizeType i = 0; i < bin_n; ++i) {
      if (l.res()[i] != r.res()[i]) {
        return false;
      }
    }
    return true;
  }
};

template <typename TArray>
inline auto headDataToString(const TArray &data, std::size_t n,
                             std::size_t head_num = 10) {
  std::stringstream ss;
  ss.precision(6);
  ss << std::fixed;
  ss << "Head data:\n";
  std::size_t i = 0;
  while (true) {
    if (n <= i) {
      break;
    }

    if (head_num <= i) {
      ss << "...";
      break;
    }

    ss << data[i] << " ";
    ++i;
  }
  return ss.str();
}

template <typename T, typename TArray, typename SizeType>
inline auto findBin(const T &data, const TArray &bin_maxes, SizeType bin_n,
                    const T &bin_min) {
  if (data < bin_min || bin_maxes[bin_n - 1] <= data) {
    return static_cast<SizeType>(-1);
  }

  auto binary_find = [](auto self, const T &data, const TArray &bin_maxes,
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

template <typename T, typename TArray, typename SizeType,
          typename SizeTypeArray>
inline auto serialImp(const TArray &data, SizeType l, SizeType r,
                      SizeTypeArray &bin_count, const TArray &bin_maxes,
                      SizeType bin_n, const T &bin_min) {
  for (SizeType i = l; i < r; ++i) {
    auto bin = findBin(data[i], bin_maxes, bin_n, bin_min);
    if (bin == static_cast<SizeType>(-1)) {
      continue;
    }
    bin_count[bin] += 1;
  }
}

// explicit instantiation

using DataTypeImp = double;
using DataTypeArrayImp = std::vector<DataTypeImp>;
using SizeTypeImp = std::size_t;
using SizeTypeArrayImp = std::vector<SizeTypeImp>;
using TaskTypeImp =
    HistogramBinTask<DataTypeImp, DataTypeArrayImp, SizeTypeImp>;
using ResultTypeImp = HistogramBinResult<SizeTypeImp, SizeTypeArrayImp>;

SizeTypeArrayImp mpiImp(int my_rank, int size, const TaskTypeImp &task,
                        const DataTypeArrayImp &data,
                        bool printDataCopyTime = false);

SizeTypeArrayImp threadImp(int num_threads, const TaskTypeImp &task,
                           const DataTypeArrayImp &data);

SizeTypeArrayImp ompImp(int num_threads, const TaskTypeImp &task,
                        const DataTypeArrayImp &data);

} // namespace homework::histogram_bin

#endif // __PROJECT_NAME_HISTOGRAM_BIN_H__
