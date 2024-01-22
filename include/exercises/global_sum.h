#ifndef __PROJECT_NAME_GLOBAL_SUM_H__
#define __PROJECT_NAME_GLOBAL_SUM_H__

#include <cstddef>
#include <vector>

namespace exercises::global_sum {

class TestCaseSerialArray {
 public:
  TestCaseSerialArray() = delete;
  TestCaseSerialArray(const TestCaseSerialArray&) = delete;
  TestCaseSerialArray(TestCaseSerialArray&&) = delete;
  TestCaseSerialArray& operator=(const TestCaseSerialArray&) = delete;
  TestCaseSerialArray& operator=(TestCaseSerialArray&&) = delete;

  static const TestCaseSerialArray& instance(std::size_t n = 0) {
    static TestCaseSerialArray instance(0, n);
    return instance;
  }

  const std::vector<double>& data() const { return _data; }

 private:
  TestCaseSerialArray(std::size_t l, std::size_t r) : _data(r - l) {
    for (std::size_t i = l; i < r; ++i) {
      _data[i - l] = l + i;
    }
  }

  std::vector<double> _data;
};

// TODO(franzero): need a thread sleep function

double computeNextValueWithRandomSleep(std::size_t x);

double serialImp(std::size_t l, std::size_t r);

double mpiImp(int my_rank, int size, std::size_t n);

double mpiAllReduceImp(int my_rank, int size, std::size_t n);

double ompImp(int num_threads, std::size_t l, std::size_t r);

double threadImp(int num_threads, std::size_t l, std::size_t r);

}  // namespace exercises::global_sum

#endif  // __PROJECT_NAME_GLOBAL_SUM_H__
