#ifndef __ConcurrencyAction_THREAD_POOL_H__
#define __ConcurrencyAction_THREAD_POOL_H__

#include <atomic>
#include <functional>
#include <future>
#include <memory>
#include <queue>

#include "thread_safe_queue.h"

namespace exercises::t_s_s {
class ThreadPool {
 public:
  explicit ThreadPool(
      unsigned int num_threads = std::thread::hardware_concurrency());

  ThreadPool(const ThreadPool&) = delete;

  ThreadPool& operator=(const ThreadPool&) = delete;

  ~ThreadPool();

  template <typename Func, typename... Arg>
  auto submit(Func&& func, Arg&&... arg);

  void runPendingTask();

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

  using LocalQueue = std::queue<FuncWrapper>;
  inline thread_local static std::unique_ptr<LocalQueue> local_work_queue{};

  std::atomic<bool> _done;
  ThreadSafeQueue<FuncWrapper> _work_queue;
  std::vector<std::thread> _threads;
  JoinGuard _join_guard;

  void workerThread() {
    // if wait to use local_work_queue, uncomment the following line. Be
    // careful, this will cause the program to live lock

    // local_work_queue = std::make_unique<LocalQueue>();
    while (!_done) {
      runPendingTask();
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
  if (local_work_queue) {
    local_work_queue->emplace(
        std::make_unique<FuncWrapper::FuncType<decltype(task)>>(
            std::move(task)));
    return future;
  }

  _work_queue.push(
      FuncWrapper{std::make_unique<FuncWrapper::FuncType<decltype(task)>>(
          std::move(task))});
  return future;
}

inline void ThreadPool::runPendingTask() {
  FuncWrapper task;

  // First, try to take a task from the local queue
  if (local_work_queue && !local_work_queue->empty()) {
    task = std::move(local_work_queue->front());
    local_work_queue->pop();
    task();
    return;
  }

  if (_work_queue.tryPop(task)) {
    task();
  } else {
    std::this_thread::yield();
  }
}
}  // namespace exercises::t_s_s

#endif  // __ConcurrencyAction_THREAD_POOL_H__
