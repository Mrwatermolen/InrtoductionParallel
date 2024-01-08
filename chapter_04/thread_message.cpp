#include <chrono>
#include <iostream>
#include <memory>
#include <mutex>
#include <random>
#include <semaphore>
#include <string>
#include <thread>
#include <vector>

static std::mutex stdout_mutex;

void busySendMsg(int my_rank, int size, std::vector<std::string> &msg_arr) {
  int dst = (my_rank + 1) % size;
  std::string msg = "Hello from " + std::to_string(my_rank);

  std::default_random_engine generator;
  std::uniform_int_distribution<int> dis(100, 300);
  std::this_thread::sleep_for(std::chrono::milliseconds(dis(generator)));

  // notify dst
  msg_arr[dst] = msg;

  // wait msg
  while (msg_arr[my_rank].empty()) {
  }

  // print msg
  std::scoped_lock lock(stdout_mutex);
  std::cout << "Thread " << my_rank << " received: " << msg_arr[my_rank] << '\n'
            << std::flush;
}

void semaphoreSendMsg(
    int my_rank, int size, std::vector<std::string> &msg_arr,
    std::vector<std::unique_ptr<std::binary_semaphore>> &smp_arr) {
  int dst = (my_rank + 1) % size;
  std::string msg = "Hello from " + std::to_string(my_rank);

  std::default_random_engine generator;
  std::uniform_int_distribution<int> dis(100, 300);
  std::this_thread::sleep_for(std::chrono::milliseconds(dis(generator)));

  // notify dst
  msg_arr[dst] = msg;
  smp_arr[dst]->release();

  // wait msg
  smp_arr[my_rank]->acquire();

  // print msg
  stdout_mutex.lock();
  std::cout << "Thread " << my_rank << " received: " << msg_arr[my_rank] << '\n'
            << std::flush;
  stdout_mutex.unlock();
}

void sendMsg() {
  std::cout << "Input number of threads: (max: "
            << std::thread::hardware_concurrency() << ")\n";
  int num_threads;
  std::cin >> num_threads;
  std::vector<std::string> msg_arr;
  msg_arr.reserve(num_threads);
for (int i = 0; i < num_threads; ++i) {
    msg_arr.emplace_back("");
  }

  std::vector<std::thread> t(num_threads);
  for (int i = 0; i < num_threads; ++i) {
    t[i] = std::thread(busySendMsg, i, num_threads, std::ref(msg_arr));
  }
  for (int i = 0; i < num_threads; ++i) {
    t[i].join();
  }

  std::cout << "=====================\n";

  std::vector<std::unique_ptr<std::binary_semaphore>> smp_arr;
  smp_arr.reserve(num_threads);
for (int i = 0; i < num_threads; ++i) {
    smp_arr.emplace_back(std::make_unique<std::binary_semaphore>(0));
  }
  for (int i = 0; i < num_threads; ++i) {
    t[i] = std::thread(semaphoreSendMsg, i, num_threads, std::ref(msg_arr),
                       std::ref(smp_arr));
  }

  for (int i = 0; i < num_threads; ++i) {
    t[i].join();
  }
}

int main() {
  sendMsg();
  return 0;
}
