#ifndef __PROJECT_NAME_HOMEWORK_H__
#define __PROJECT_NAME_HOMEWORK_H__

#include <chrono>
#include <cmath>
#include <cstddef>
#include <iostream>
#include <random>
#include <sstream>
#include <utility>
#include <vector>

#include "helper.h"

namespace homework {
class Task{
  public:
  Task() = default;

  explicit Task(std::size_t n) : _n(n) {}

  Task(const Task& other) = default;

  Task(Task&& other) noexcept = default;

  Task& operator=(const Task& other) = default;

  Task& operator=(Task&& other) noexcept = default;

  virtual ~Task() = default;

  auto n() const { return _n; }

  auto& n() {return _n;}

  virtual std::string toString() const = 0;

  virtual std::size_t bytes() const = 0;

  private:
  std::size_t _n{};
};

template<typename T>
class Result{
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

// namespace carlo_pi {

// struct CarloPITask {
//   std::size_t _n;
//   double _radius;

//   auto n() const { return _n; }

//   auto radius() const {return _radius; }

//   std::string toString() const {
//     std::stringstream ss;
//     ss << "Task: " << n();
//     return ss.str();
//   }

//   std::size_t bytes() const { return sizeof(*this); }

//   static auto createFromInput() {
//     auto n = getInoutOneDimensionProblemSize();

//     double r;
//     std::cout << "Input radius:\n";
//     std::cin >> r;

//     return CarloPITask{n, r};
//   }
// };

// struct CarloPIResult {
//   double _res;

//   explicit CarloPIResult(double res) : _res(res) {}

//   CarloPIResult(std::size_t counter, std::size_t n)
//       : _res(4 * static_cast<double>(counter) / static_cast<double>(n)) {}

//   auto &&res() { return _res; }

//   const auto &res() const { return _res; }

//   std::string toString(int precision = 6) const {
//     std::stringstream ss;
//     ss.precision(precision);
//     ss << std::fixed;

//     ss << "Value: " << _res;

//     return ss.str();
//   }

//   inline static double epsilon = 1e-4;
//   static auto sameResult(const CarloPIResult &l, const CarloPIResult &r) {
//     return l.res() == r.res() || std::abs(l.res() - r.res()) < epsilon;
//   }
// };

// inline auto randomPoint(double l, double r) {
//   static std::random_device rd;
//   static std::mt19937 gen(rd());
//   static std::uniform_real_distribution<> dis(l,r);
//   return std::make_pair(dis(gen),dis(gen));
// }

// auto serialImp(std::size_t n, double radius) {
//   auto circle = [&](double x, double y){
//     auto dis = x*x+y*y;
//     return dis <= radius * radius;
//   };

//   std::size_t counter = 0;
//   for(std::size_t i = 0; i < n; ++i) {
//     auto [x,y] = randomPoint(-radius, radius);
//     if (circle(x,y)) {
//       ++counter;
//     }
//   }
//   return counter;
// }

// std::size_t mpiImp(int my_rank, int size, const CarloPITask &task, bool printDataCopyTime = false);

// // std::size_t threadImp(int num_threads, const CarloPITask &task);

// // std::size_t ompImp(int num_threads, const CarloPITask &task);

// } // namespace carlo_pi

#endif  // __PROJECT_NAME_HOMEWORK_H__
