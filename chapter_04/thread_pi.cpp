#include <chrono>
#include <cstddef>
#include <iostream>
#include <mutex>
#include <stdexcept>
#include <thread>

double serialPi(std::size_t l, std::size_t r) {
  double factor = (l % 2) != 0U ? -1.0 : 1.0;
  double sum = 0;
  for (auto i = l; i < r; ++i) {
    sum += factor / (2 * i + 1);
    factor = -factor;
  }
  return sum * 4;
}

double busyWaitPi(std::size_t n) {
  double busy_wait_pi = 0;
  int flag = 0;
  auto f = [&busy_wait_pi, &flag](std::size_t l, std::size_t r, int my_rank,
                                  int size) {
    double sum = serialPi(l, r);
    while (my_rank != flag) {
    }
    busy_wait_pi += sum;
    flag = (flag + 1) % size;
  };
  std::cout << "Input number of threads: (max: "
            << std::thread::hardware_concurrency() << ")\n";
  int num_threads;
  std::cin >> num_threads;
  if (n % num_threads != 0) {
    std::cout << "n must be divisible by num_threads\n";
    throw std::runtime_error("n must be divisible by num_threads");
  }
  std::thread t[num_threads];
  for (int i = 0; i < num_threads; ++i) {
    auto l = i * (n / num_threads);
    auto r = (i + 1) * (n / num_threads);
    t[i] = std::thread(f, l, r, num_threads - 1 - i, num_threads);
  }

  for (int i = 0; i < num_threads; ++i) {
    t[i].join();
  }

  return busy_wait_pi;
}

double mutexPi(std::size_t n) {
  double mutex_pi = 0;
  std::mutex mutex;
  auto f = [&mutex_pi, &mutex](std::size_t l, std::size_t r) {
    double sum = serialPi(l, r);

    std::scoped_lock lock(mutex);
    mutex_pi += sum;
  };
  std::cout << "Input number of threads: (max: "
            << std::thread::hardware_concurrency() << ")\n";
  int num_threads;
  std::cin >> num_threads;
  if (n % num_threads != 0) {
    std::cout << "n must be divisible by num_threads\n";
    throw std::runtime_error("n must be divisible by num_threads");
  }
  std::thread t[num_threads];
  for (int i = 0; i < num_threads; ++i) {
    auto l = i * (n / num_threads);
    auto r = (i + 1) * (n / num_threads);
    t[i] = std::thread(f, l, r);
  }

  for (int i = 0; i < num_threads; ++i) {
    t[i].join();
  }

  return mutex_pi;
}

void calculatePi() {
  auto serial_pi = [](std::size_t l, std::size_t r, double factor) {
    double sum = 0;
    for (auto i = l; i < r; ++i) {
      sum += factor / (2 * i + 1);
      factor = -factor;
    }
    return sum * 4;
  };

  std::cout << "Input n: \n";
  std::size_t n;
  std::cin >> n;

  auto start = std::chrono::steady_clock::now();
  auto serial_pi_result = serial_pi(0, n, 1.0);
  auto end = std::chrono::steady_clock::now();
  auto serial_time_ms =
      std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
          .count();
  std::cout << "Serial pi: " << serial_pi_result << "\n";
  std::cout << "Elapsed time: " << serial_time_ms << " ms\n";

  // busy wait
  start = std::chrono::steady_clock::now();
  auto busy_wait_pi = busyWaitPi(n);
  end = std::chrono::steady_clock::now();
  auto busy_wait_time_ms =
      std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
          .count();
  std::cout << "Busy wait pi: " << busy_wait_pi << "\n";
  std::cout << "Elapsed time: " << busy_wait_time_ms << " ms\n";

  // mutex
  start = std::chrono::steady_clock::now();
  auto mutex_pi = mutexPi(n);
  end = std::chrono::steady_clock::now();
  auto mutex_time_ms =
      std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
          .count();
  std::cout << "Mutex pi: " << mutex_pi << "\n";
  std::cout << "Elapsed time: " << mutex_time_ms << " ms\n";
}

int main(int argc, char **argv) {
  calculatePi();
  return 0;
}
