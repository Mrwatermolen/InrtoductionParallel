#ifndef __PROJECT_NAME_HOMEWORK_H__
#define __PROJECT_NAME_HOMEWORK_H__

#include <cstddef>
#include <string>
#include <utility>

namespace homework {
class Task {
 public:
  Task() = default;

  explicit Task(std::size_t n) : _n(n) {}

  Task(const Task& other) = default;

  Task(Task&& other) noexcept = default;

  Task& operator=(const Task& other) = default;

  Task& operator=(Task&& other) noexcept = default;

  virtual ~Task() = default;

  auto n() const { return _n; }

  auto& n() { return _n; }

  virtual std::string toString() const = 0;

  virtual std::size_t bytes() const = 0;

 private:
  std::size_t _n{};
};

template <typename T>
class Result {
 public:
  Result() = default;

  explicit Result(T res) : _res(std::move(res)) {}

  Result(const Result& other) = default;

  Result(Result&& other) noexcept = default;

  Result& operator=(const Result& other) = default;

  Result& operator=(Result&& other) noexcept = default;

  virtual ~Result() = default;

  auto& res() { return _res; }

  const auto& res() const { return _res; }

  virtual std::string toString() const = 0;

 private:
  T _res{};
};
}  // namespace homework

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

#endif  // __PROJECT_NAME_HOMEWORK_H__
