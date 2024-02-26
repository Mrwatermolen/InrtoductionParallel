#ifndef __PROJECT_NAME_POOL_H__
#define __PROJECT_NAME_POOL_H__

#include <atomic>
#include <functional>
#include <future>

#include "thread_safe_queue.h"

namespace exercises::t_s_s::bug_version {

class ThreadPool {
 public:
  explicit ThreadPool(
      unsigned int num_threads = std::thread::hardware_concurrency());

  ThreadPool(const ThreadPool&) = delete;

  ThreadPool& operator=(const ThreadPool&) = delete;

  ~ThreadPool();

  template <typename Func, typename... Arg>
  auto submit(Func&& func, Arg&&... arg);

 private:
  struct FuncWrapper {
    struct FuncBase {
      virtual void call() = 0;
      virtual ~FuncBase() = default;
    };

    template <typename T>
    struct FuncType : FuncBase {
      T _func;
      explicit FuncType(T&& func) : _func(std::move(func)) {}
      ~FuncType() override = default;

      void call() override { _func(); }
    };

    std::unique_ptr<FuncBase> _func;

    void operator()() { _func->call(); }
  };

  struct JoinGuard {
    std::vector<std::thread>& _threads;
    explicit JoinGuard(std::vector<std::thread>& threads) : _threads(threads) {}
    ~JoinGuard() {
      for (auto& thread : _threads) {
        if (thread.joinable()) {
          thread.join();
        }
      }
    }
  };

  std::atomic<bool> _done;
  ThreadSafeQueue<FuncWrapper> _work_queue;
  std::vector<std::thread> _threads;
  JoinGuard _join_guard;

  void workerThread() {
    while (!_done) {
      FuncWrapper task;
      if (_work_queue.tryPop(task)) {
        task();
      } else {
        std::this_thread::yield();
      }
    }
  }
};

ThreadPool::ThreadPool(unsigned int num_threads)
    : _done(false), _threads(num_threads), _join_guard(_threads) {
  if (num_threads == 0) {
    throw std::invalid_argument("num_threads == 0");
  }

  try {
    for (unsigned int i = 0; i < num_threads; ++i) {
      _threads.emplace_back(&ThreadPool::workerThread, this);
    }
  } catch (const std::exception& e) {
    _done = true;
    throw std::runtime_error("ThreadPool::ThreadPool() failed. Exception: " +
                             std::string(e.what()));
  }
}

ThreadPool::~ThreadPool() { _done = true; }

template <typename Func, typename... Arg>
inline auto ThreadPool::submit(Func&& func, Arg&&... arg) {
  // using RetType = std::invoke_result_t<Func, Arg...>;
  using FuncRetType = decltype(func(arg...));
  using TaskType = std::packaged_task<FuncRetType()>;

  auto task =
      TaskType(std::bind(std::forward<Func>(func), std::forward<Arg>(arg)...));
  auto future = task.get_future();
  _work_queue.push(
      FuncWrapper{std::make_unique<FuncWrapper::FuncType<decltype(task)>>(
          std::move(task))});
  return future;
}

}  // namespace exercises::t_s_s::bug_version

#endif  // __PROJECT_NAME_POOL_H__
