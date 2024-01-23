#ifndef __ConcurrencyAction_THREAD_SAFE_QUEUE_H__
#define __ConcurrencyAction_THREAD_SAFE_QUEUE_H__

#include <algorithm>
#include <condition_variable>
#include <exception>
#include <memory>
#include <mutex>
#include <queue>

namespace exercises::t_s_s {

class EmptyQueue : public std::exception {
 public:
  const char *what() const noexcept override { return "empty queue"; }
};

template <typename T>
class ThreadSafeQueue {
 public:
  ThreadSafeQueue() = default;

  ThreadSafeQueue(const ThreadSafeQueue<T> &other);

  // ThreadSafeQueue(ThreadSafeQueue<T> &&) = default;

  // ThreadSafeQueue<T> &operator=(const ThreadSafeQueue<T> &) = delete;

  // ThreadSafeQueue<T> &operator=(ThreadSafeQueue<T> &&) = default;

  ~ThreadSafeQueue() = default;

  auto empty() const;

  auto size() const;

  auto push(T value);

  auto pop(T &value);

  auto waitAndPop(T &value);

  auto tryPop(T &value) -> bool;

  auto pop() -> std::unique_ptr<T>;

  auto waitAndPop() -> std::unique_ptr<T>;

  auto tryPop() -> std::unique_ptr<T>;

 private:
  mutable std::mutex _m{};
  std::condition_variable _cond{};
  std::queue<T> _data{};
};

template <typename T>
inline ThreadSafeQueue<T>::ThreadSafeQueue(const ThreadSafeQueue<T> &other) {
  std::scoped_lock lock{other._m};
  _data = other._data;
}

template <typename T>
inline auto ThreadSafeQueue<T>::empty() const {
  std::scoped_lock lock{_m};
  return _data.empty();
}

template <typename T>
inline auto ThreadSafeQueue<T>::size() const {
  std::scoped_lock lock{_m};
  return _data.size();
}

template <typename T>
inline auto ThreadSafeQueue<T>::push(T value) {
  std::scoped_lock lock{_m};
  _data.push(std::move(value));
  _cond.notify_one();
}

template <typename T>
inline auto ThreadSafeQueue<T>::pop(T &value) {
  std::scoped_lock lock{_m};
  if (_data.empty()) {
    throw EmptyQueue{};
  }
  value = std::move(_data.front());
  _data.pop();
}

template <typename T>
inline auto ThreadSafeQueue<T>::waitAndPop(T &value) {
  std::unique_lock lock{_m};
  _cond.wait(lock, [this] { return !_data.empty(); });
  try {
    value = std::move(_data.front());
    _data.pop();
  } catch (const std::exception &e) {
    _cond.notify_one();
    throw e;
  }
}

template <typename T>
inline auto ThreadSafeQueue<T>::tryPop(T &value) -> bool {
  std::scoped_lock lock{_m};
  if (_data.empty()) {
    return false;
  }
  value = std::move(_data.front());
  _data.pop();
  return true;
}

template <typename T>
inline auto ThreadSafeQueue<T>::pop() -> std::unique_ptr<T> {
  std::scoped_lock lock{_m};
  if (_data.empty()) {
    throw EmptyQueue{};
  }
  auto res = std::make_unique<T>(std::move(_data.front()));
  _data.pop();
  return res;
}

template <typename T>
inline auto ThreadSafeQueue<T>::waitAndPop() -> std::unique_ptr<T> {
  std::unique_lock lock{_m};
  _cond.wait(lock, [this] { return !_data.empty(); });
  try {
    auto res = std::make_unique<T>(std::move(_data.front()));
    _data.pop();
    return res;
  } catch (const std::exception &e) {
    _cond.notify_one();
    throw e;
  }
}

template <typename T>
inline auto ThreadSafeQueue<T>::tryPop() -> std::unique_ptr<T> {
  std::scoped_lock lock{_m};
  if (_data.empty()) {
    return std::unique_ptr<T>{};
  }
  auto res = std::make_unique<T>(std::move(_data.front()));
  _data.pop();
  return res;
}

}  // namespace exercises::t_s_s

#endif  // __ConcurrencyAction_THREAD_SAFE_QUEUE_H__
