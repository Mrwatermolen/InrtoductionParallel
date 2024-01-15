#ifndef __PROJECT_NAME_TRAP_INTEGRAL_H__
#define __PROJECT_NAME_TRAP_INTEGRAL_H__

#include <cmath>
#include <utility>

#include "helper.h"
#include "homework.h"

namespace homework::trap_integral {

template <typename T>
class TrapIntegralTask : public Task {
 public:
  TrapIntegralTask() = default;

  TrapIntegralTask(T l, T r, std::size_t n) : _l(l), _r(r), _n(n) {}

  ~TrapIntegralTask() override = default;

  T _l{};
  T _r{};
  std::size_t _n{};

  auto &l() { return _l; }

  auto &r() { return _r; }

  auto &n() { return _n; }

  const auto &l() const { return _l; }

  const auto &r() const { return _r; }

  const auto &n() const { return _n; }

  std::string toString() const override {
    std::stringstream ss;
    ss << "Task: "
       << "sample points: " << _n << " domain: [" << _l << ", " << _r << ")";
    return ss.str();
  }

  std::size_t bytes() const override { return sizeof(*this); }

  static auto createFromInput() {
    T l{};
    T r{};

    auto n = getInoutOneDimensionProblemSize();
    std::cout << "Input l, r:\n";
    std::cin >> l >> r;

    return TrapIntegralTask{l, r, n};
  }
};

template <typename T>
class TrapIntegralResult : public Result<T> {
 public:
  TrapIntegralResult() = default;

  explicit TrapIntegralResult(T res) : Result<T>(std::move(res)) {}

  ~TrapIntegralResult() override = default;

  inline static int precision = 6;
  std::string toString() const override {
    std::stringstream ss;
    ss.precision(precision);
    ss << std::fixed;
    ss << "Value: " << this->res();

    return ss.str();
  }

  inline static T epsilon = 1e-6;
  static auto sameResult(const TrapIntegralResult &l,
                         const TrapIntegralResult &r) {
    return l.res() == r.res() || std::abs(l.res() - r.res()) < epsilon;
  }
};

template <typename T>
inline auto givenFunc(const T &x)
    -> decltype(std::sin(x) * std::cos(x) * std::exp(-x)) {
  return std::sin(x) * std::cos(x) * std::exp(-x);
}

template <typename T>
inline auto givenFuncDerivative(const T &x)
    -> decltype(-std::exp(-x) * (0.5 * std::sin(2 * x) + std::cos(2 * x))) {
  return std::exp(-x) * (-0.5 * std::sin(2 * x) + std::cos(2 * x));
}

template <typename T, typename SizeType, typename Func>
inline auto serialImp(const T &l, const T &r, SizeType n, Func &&f) {
  auto h = (r - l) / n;
  auto sum = (f(l) + f(r)) / 2;
  for (int i = 1; i < n; ++i) {
    sum += f(l + i * h);
  }
  return sum * h;
}

using TrapIntegralDataTypeImp = double;
using TrapIntegralTaskImp = TrapIntegralTask<TrapIntegralDataTypeImp>;
using TrapIntegralResultImp = TrapIntegralResult<TrapIntegralDataTypeImp>;

TrapIntegralDataTypeImp mpiImp(int my_rank, int size, TrapIntegralTaskImp &task,
                               bool printDataCopyTime = false);

TrapIntegralDataTypeImp threadImp(int num_threads,
                                  const TrapIntegralTaskImp &task);

TrapIntegralDataTypeImp ompImp(int num_threads,
                               const TrapIntegralTaskImp &task);

}  // namespace homework::trap_integral

#endif  // __PROJECT_NAME_TRAP_INTEGRAL_H__
