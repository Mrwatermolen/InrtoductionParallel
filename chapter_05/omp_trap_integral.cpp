#include <omp.h>

#include <chrono>
#include <iostream>

#include "helper.h"

double trap(double l, double r, long long n, double (*f)(double));

double ompImp(double l, double r, long long n, int num_threads,
              double (*f)(double));

int main(int argc, char** argv) {
  if (2 < argc) {
    std::cout << "Usage: " << argv[0] << "<n> \n";
    return 1;
  }
  int num_threads = 1;
  if (argc == 2) {
    num_threads = std::stoi(argv[1]);
    if (num_threads < 1) {
      std::cerr << "num_threads must be greater than 0\n";
      return 1;
    }
  }

  double a = 0;
  double b = 0;
  long long n = 0;

  std::cout << "Enter a, b, and n\n";
  std::cin >> a >> b >> n;

  if (b <= a) {
    std::cerr << "a must be less than b\n";
    return 1;
  }

  if (n < 1) {
    std::cerr << "n must be greater than 0\n";
    return 1;
  }

  auto f = [](double x) { return x * x; };

  std::cout << "Calculate Integral of x^2 domain: [" << a << ", " << b
            << "] with n: " << n << "\n";

  try {
    std::cout << "OpenMP Configure: " << num_threads << "\n";
    auto [omp_time, omp_res] = measureTime(ompImp, a, b, n, num_threads, f);
    std::cout << "OpenMP Elapsed Time: "
              << std::chrono::duration_cast<std::chrono::milliseconds>(omp_time)
                     .count()
              << " ms\n";
    std::cout << "OpenMP Imp: " << omp_res << "\n";
  } catch (std::exception& e) {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  auto omp_trap = [](double l, double r, long long n, int num_threads,
                     double (*f)(double)) {
    auto h = (r - l) / n;
    r = l + h * n;
    double sum = 0;
    sum += (f(l) + f(r)) / 2.0;

#pragma omp parallel for num_threads(num_threads) reduction(+ : sum)
    for (decltype(n) i = 1; i < n; ++i) {
      sum += f(l + i * h);
    }

    return sum * h;
  };

  auto [omp_trap_time, omp_trap_res] =
      measureTime(omp_trap, a, b, n, num_threads, f);
  auto omp_trap_time_ms =
      std::chrono::duration_cast<std::chrono::milliseconds>(omp_trap_time);
  std::cout << "OpenMP Trap Elapsed Time: " << omp_trap_time_ms.count()
            << " ms\n";
  std::cout << "OpenMP Trap Imp: " << omp_trap_res << "\n";

  auto [serial_time, serial_res] = measureTime(trap, a, b, n, f);
  auto serial_time_ms =
      std::chrono::duration_cast<std::chrono::milliseconds>(serial_time);
  std::cout << "Serial Imp: " << serial_res << "\n";
  std::cout << "Serial Elapsed Time: " << serial_time_ms.count() << " ms\n";

  auto speed_up = speedUp(
      std::chrono::duration_cast<std::chrono::duration<double>>(serial_time)
          .count(),
      std::chrono::duration_cast<std::chrono::duration<double>>(omp_trap_time)
          .count());
  std::cout << "Speed Up: " << speed_up << "\n";
  std::cout << "Efficiency: " << efficiency(speed_up, num_threads) << "\n";
  return 0;
}

double ompImp(const double l, const double r, const long long n,
              const int num_threads, double (*f)(double)) {
  const auto h = (r - l) / n;
  const auto strip = n / num_threads;
  if (n % num_threads != 0) {
    throw std::invalid_argument("n must be divisible by num_threads");
  }
  double global_res = 0;

#pragma omp parallel num_threads(num_threads)
  {
    auto my_rank = omp_get_thread_num();
    auto size = omp_get_num_threads();

    auto local_l = l + my_rank * strip * h;
    auto local_r = local_l + strip * h;
    auto local_n = strip;
    auto local_res = trap(local_l, local_r, local_n, f);

#pragma omp critical
    { global_res += local_res; }
  }

  return global_res;
}

double trap(double l, double r, long long n, double (*f)(double)) {
  auto h = (r - l) / n;
  r = l + h * n;
  double sum = 0;
  sum += (f(l) + f(r)) / 2.0;
  for (decltype(n) i = 1; i < n; ++i) {
    sum += f(l + i * h);
  }
  sum = sum * h;
  return sum;
}
