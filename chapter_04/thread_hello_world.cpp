#include <iostream>
#include <thread>

int main() {
  std::cout << "Max threads: " << std::thread::hardware_concurrency() << "\n";
  auto print_hello = [](int id) {
    std::cout << "Hello from thread " << id << "\n";
  };

  auto max = std::thread::hardware_concurrency();
  for (int i = 0; i < max; ++i) {
    std::thread t(print_hello, i);
    t.join();
  }
}
