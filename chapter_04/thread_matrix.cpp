#include <chrono>
#include <iostream>
#include <thread>
#include <vector>

void localMatVec(int my_rank, int size, const double *A, const double *x,
                 double *y, int m, int n) {
  int local_m = m / size;
  int l = my_rank * local_m;
  int r = (my_rank + 1) * local_m;
  for (int i = l; i < r; ++i) {
    y[i] = 0;
    for (int j = 0; j < n; ++j) {
      y[i] += A[i * n + j] * x[j];
    }
  }
}

void threadMatVec(int m, int n, int num_thread) {
  if (m % num_thread != 0) {
    return;
  }
  std::cout << "Configure: m: " << m << " n: " << n << " t: " << num_thread
            << "\n";
  std::vector<std::thread> t_vec;
  auto a = new double[m * n];
  auto x = new double[n];
  auto y = new double[m];

  for (int i = 0; i < m; ++i) {
    y[i] = 0;
    for (int j = 0; j < n; ++j) {
      a[i * n + j] = i + j;
    }
  }
  for (int i = 0; i < n; ++i) {
    x[i] = i + 1;
  }

  t_vec.reserve(num_thread);
  for (int i = 0; i < num_thread; ++i) {
    t_vec.emplace_back(localMatVec, i, num_thread, a, x, y, m, n);
  }

  for (auto &&t : t_vec) {
    t.join();
  }

  delete[] x;
  delete[] y;
  delete[] a;
}

int main(int argc, char **argv) {
  int m;
  int n;
  int num_thread;

  std::cout << "Input m, n, thread\n";
  std::cin >> m >> n >> num_thread;
  auto start = std::chrono::high_resolution_clock::now();
  threadMatVec(m, n, num_thread);
  auto end = std::chrono::high_resolution_clock::now();
  auto parallel_duration = end - start;
  start = std::chrono::high_resolution_clock::now();
  threadMatVec(m, n, 1);
  end = std::chrono::high_resolution_clock::now();
  auto serial_duration = end - start;
  std::cout << "Parallel : "
            << std::chrono::duration_cast<std::chrono::milliseconds>(
                   parallel_duration)
                   .count()
            << " ms\n";
  std::cout << "Serial   : "
            << std::chrono::duration_cast<std::chrono::milliseconds>(
                   serial_duration)
                   .count()
            << " ms\n";
}
