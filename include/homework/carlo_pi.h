#ifndef __PROJECT_NAME_CARLO_PI_H__
#define __PROJECT_NAME_CARLO_PI_H__

#include <chrono>
#include <cmath>
#include <cstddef>
#include <iostream>
#include <random>
#include <sstream>
#include <utility>
#include <vector>

#include "helper.h"
#include "homework.h"

namespace homework::carlo_pi {

template<typename T>
struct CarloPITask: public Task {
  CarloPITask() = default;

  explicit CarloPITask(std::size_t n, T radius) : Task(n), _radius(radius) {}

  ~CarloPITask() override = default;

  T _radius{};

  const auto &radius() const { return _radius; }

  auto &radius() { return _radius; }

  std::string toString() const override {
    std::stringstream ss;
    ss << "Task: " << " samples: " << this->n() << " radius: " << this->radius() << "\n";
    return ss.str();
  }

  constexpr std::size_t bytes() const override { return sizeof(*this); }

  static auto createFromInput() {
    auto n = getInoutOneDimensionProblemSize();

    T r;
    std::cout << "Input radius:\n";
    std::cin >> r;

    return CarloPITask{n, r};
  }
};

template<typename T>
struct CarloPIResult: public Result<T> {

  explicit CarloPIResult(T res) : Result<T>(std::move(res)) {}

  CarloPIResult(std::size_t counter, std::size_t n)
      : Result<T>(4 * static_cast<T>(counter) / static_cast<T>(n)) {}

  inline static int precision = 6;
  std::string toString() const override {
    std::stringstream ss;
    ss.precision(precision);
    ss << std::fixed;

    ss << "Value: " << this->res();

    return ss.str();
  }

  inline static double epsilon = 1e-4;
  static auto sameResult(const CarloPIResult &l, const CarloPIResult &r) {
    return l.res() == r.res() || std::abs(l.res() - r.res()) < epsilon;
  }
};


template<typename T>
inline auto randomPoint(T l, T r) {
  static std::random_device rd;
  static std::mt19937 gen(rd());
  static std::uniform_real_distribution<> dis(l,r);
  return std::make_pair(dis(gen),dis(gen));
}


template<typename T>
auto serialImp(std::size_t n, T radius) {
  auto circle = [&](T x, T y){
    auto dis = x*x+y*y;
    return dis <= radius * radius;
  };

  std::size_t counter = 0;
  for(std::size_t i = 0; i < n; ++i) {
    auto [x,y] = randomPoint(-radius, radius);
    if (circle(x,y)) {
      ++counter;
    }
  }
  return counter;
}

using DataTypeImp = double;
using TaskTypeImp = CarloPITask<DataTypeImp>;
using ResultTypeImp = CarloPIResult<DataTypeImp>;

std::size_t mpiImp(int my_rank, int size, const TaskTypeImp &task, bool printDataCopyTime = false);

std::size_t threadImp(int num_threads, const TaskTypeImp &task);

std::size_t ompImp(int num_threads, const TaskTypeImp &task);

}  // namespace homework::carlo_pi

#endif // __PROJECT_NAME_CARLO_PI_H__
