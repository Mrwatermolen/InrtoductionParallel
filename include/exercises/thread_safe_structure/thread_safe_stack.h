#ifndef __ConcurrencyAction_THREAD_SAFE_STACK_H__
#define __ConcurrencyAction_THREAD_SAFE_STACK_H__

#include <algorithm>
#include <memory>
#include <mutex>
#include <stack>

namespace exercises::t_s_s {

class EmptyStack : public std::exception {
 public:
  const char *what() const noexcept override { return "empty stack"; }
};

template <typename T>
class ThreadSafeStack {
 public:
  ThreadSafeStack() = default;

  ThreadSafeStack(const ThreadSafeStack<T> &other);

  // ThreadSafeStack(ThreadSafeStack<T> &&);

  // ThreadSafeStack<T> &operator=(const ThreadSafeStack<T> &) = delete;

  // ThreadSafeStack<T> &operator=(ThreadSafeStack<T> &&);

  ~ThreadSafeStack() = default;

  auto empty() const -> bool;

  auto push(T value) -> void;

  auto pop() -> std::unique_ptr<T>;

  auto pop(T &value) -> void;

 private:
  mutable std::mutex _m{};
  std::stack<T> _data{};
};

template <typename T>
ThreadSafeStack<T>::ThreadSafeStack(const ThreadSafeStack<T> &other) {
  std::scoped_lock lock{other._m};
  _data = other._data;
}

template <typename T>
inline auto ThreadSafeStack<T>::empty() const -> bool {
  std::scoped_lock lock{_m};
  return _data.empty();
}

template <typename T>
inline auto ThreadSafeStack<T>::push(T value) -> void {
  std::scoped_lock lock{_m};
  _data.push(std::move(value));
}

template <typename T>
inline auto ThreadSafeStack<T>::pop() -> std::unique_ptr<T> {
  std::scoped_lock lock{_m};
  if (_data.empty()) {
    throw EmptyStack{};
  }

  auto res = std::make_unique<T>(std::move(_data.top()));
  _data.pop();
  return res;
}

template <typename T>
inline auto ThreadSafeStack<T>::pop(T &value) -> void {
  std::scoped_lock lock{_m};
  if (_data.empty()) {
    throw EmptyStack{};
  }

  value = std::move(_data.top());
  _data.pop();
}

}  // namespace exercises::t_s_s

#endif  // __ConcurrencyAction_THREAD_SAFE_STACK_H__
