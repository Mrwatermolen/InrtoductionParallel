#include <barrier>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <random>
#include <semaphore>
#include <stdexcept>
#include <thread>
#include <vector>

// can't be reusable. std::latch
class MyBusyBarrier {
public:
  explicit MyBusyBarrier(int counter) : _counter(counter) {}

  void wait() {
    _mutex.lock();
    --_counter;
    _mutex.unlock();
    while (0 < _counter) {
    }
  }

private:
  std::mutex _mutex{};
  int _counter;
};

// can be reusable. bug.
class MySmpBarrier {
public:
  explicit MySmpBarrier(int counter) : _thread_counter(counter) {
    if (_thread_counter <= 1) {
      throw std::runtime_error("counter must be greater than 1");
    }
  }

  void wait() {
    _mutex.lock();
    if (_counter == _thread_counter - 1) {
      _counter = 0;
      _mutex.unlock();
      for (int i = 1; i < _thread_counter; ++i) {
        _smp_barrier.release();
      }
      return;
    }

    ++_counter;
    _mutex.unlock();
    // still wait
    if (!_smp_barrier.try_acquire_for(std::chrono::seconds(10))) {
      throw std::runtime_error("timeout");
    }
    // smp_barrier.acquire();
  }

private:
  std::mutex _mutex{};
  std::counting_semaphore<> _smp_barrier{0};
  int _counter{0}, _thread_counter;
};

class MyCondBarrier {
public:
  explicit MyCondBarrier(int counter) : _thread_counter(counter) {}

  void wait() {
    std::unique_lock lock(_mutex);
    int t = _time;
    ++_counter;

    if (_counter == _thread_counter) {
      _time++;
      _counter = 0;
      _cond.notify_all();
      return;
    }

    // prevent spurious wake and lost wake
    _cond.wait(lock, [&] { return t != _time; });
  }

private:
  std::condition_variable _cond{};
  std::mutex _mutex{};
  int _counter{0};
  int _time{0};
  int _thread_counter;
};

void simpleUseBarrier() {
  std::cout << "Input number of threads: (max: "
            << std::thread::hardware_concurrency() << ")\n";
  int num_threads;
  std::cin >> num_threads;

  MyBusyBarrier bb{num_threads};
  auto do_something = [&](int my_rank) {
    std::default_random_engine e;
    std::uniform_int_distribution<int> dis(100, 300);
    std::cout << "Thread: " << my_rank << " sleep\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(dis(e)));

    std::cout << "Thread: " << my_rank << " awake\n";
    bb.wait();
    std::cout << "Thread: " << my_rank << " sync\n";
  };

  std::vector<std::thread> t;
  t.reserve(num_threads);
for (int i = 0; i < num_threads; ++i) {
    t.emplace_back(do_something, i);
  }

  for (auto &&tt : t) {
    tt.join();
  }
  std::cout << "\n";

  MySmpBarrier smp_b{num_threads};
  auto do_something_01 = [&](int my_rank) {
    std::default_random_engine e;
    std::uniform_int_distribution<int> dis(100, 300);
    std::cout << "Thread: " << my_rank << " sleep\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(dis(e)));

    std::cout << "Thread: " << my_rank << " awake\n";
    smp_b.wait();
    std::cout << "Thread: " << my_rank << " sync\n";
    std::this_thread::sleep_for(std::chrono::seconds(1));
    smp_b.wait();
    std::cout << "Thread: " << my_rank << " sync again\n";
    std::this_thread::sleep_for(std::chrono::seconds(1));

    if (my_rank == 0) {
      std::cout << "Bug in here" << "\n";
      try {
        smp_b.wait();
      } catch (std::runtime_error &e) {
        std::cout << e.what() << "\n";
        std::cout << "Thread " << my_rank << " failed to pass 0\n";
        return;
      }
      std::cout << "Thread " << my_rank << " pass 0\n";
      std::this_thread::sleep_for(std::chrono::seconds(5));
      try {
        smp_b.wait();
      } catch (std::runtime_error &e) {
        std::cout << e.what() << "\n";
        std::cout << "Thread " << my_rank << " failed to pass 1\n";
        return;
      }
      std::cout << "Thread " << my_rank << " pass 1\n";
    } else {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
      try {
        smp_b.wait();
      } catch (std::runtime_error &e) {
        std::cout << e.what() << "\n";
        std::cout << "Thread " << my_rank << " failed to pass 1\n";
        return;
      }
      std::cout << "Thread " << my_rank << " pass 0\n";
      try {
        smp_b.wait();
      } catch (std::runtime_error &e) {
        std::cout << e.what() << "\n";
        std::cout << "Thread " << my_rank << " failed to pass 1\n";
        return;
      }
      std::cout << "Thread " << my_rank << " pass 1\n";
    }
  };

  t.clear();
  for (int i = 0; i < num_threads; ++i) {
    t.emplace_back(do_something_01, i);
  }

  for (auto &&tt : t) {
    tt.join();
  }
  std::cout << '\n';

  MyCondBarrier cond_b(num_threads);
  t.clear();

  auto do_something_02 = [&](int my_rank) {
    std::default_random_engine e;
    std::uniform_int_distribution<int> dis(100, 300);
    std::cout << "Thread: " << my_rank << " sleep\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(dis(e)));

    std::cout << "Thread: " << my_rank << " awake\n";
    std::cout << std::flush;
    cond_b.wait();
    std::cout << "Thread: " << my_rank << " sync\n";
    std::cout << std::flush;
    std::this_thread::sleep_for(std::chrono::seconds(1));
    cond_b.wait();
    std::cout << "Thread: " << my_rank << " sync again\n";
    std::cout << std::flush;
    std::this_thread::sleep_for(std::chrono::seconds(1));

    if (my_rank == 0) {
      std::cout << "Not Bug" << '\n';
      cond_b.wait();
      std::cout << "Thread " << my_rank << " pass 0\n";
      std::this_thread::sleep_for(std::chrono::seconds(5));
      cond_b.wait();
      std::cout << "Thread " << my_rank << " pass 1\n";
    } else {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
      cond_b.wait();
      std::cout << "Thread " << my_rank << " pass 0\n";
      cond_b.wait();
      std::cout << "Thread " << my_rank << " pass 1\n";
    }
  };

  for (int i = 0; i < num_threads; ++i) {
    t.emplace_back(do_something_02, i);
  }

  for (auto &&tt : t) {
    tt.join();
  }

  std::cout << "Std Implementation" << '\n';

  std::barrier std_b(num_threads);
  t.clear();
  auto do_something_03 = [&](int my_rank) {
    std::default_random_engine e;
    std::uniform_int_distribution<int> dis(100, 300);
    std::cout << "Thread: " << my_rank << " sleep\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(dis(e)));

    std::cout << "Thread: " << my_rank << " awake\n";
    std::cout << std::flush;
    std_b.arrive_and_wait();
    std::cout << "Thread: " << my_rank << " sync\n";
    std::cout << std::flush;
  };

  for (int i = 0; i < num_threads; ++i) {
    t.emplace_back(do_something_03, i);
  }
  for (auto &&tt : t) {
    tt.join();
  }
}

int main() {
  simpleUseBarrier();
  return 0;
}
