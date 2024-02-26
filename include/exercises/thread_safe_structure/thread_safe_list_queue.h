#ifndef __ConcurrencyAction_THREAD_SAFE_LIST_QUEUE_H__
#define __ConcurrencyAction_THREAD_SAFE_LIST_QUEUE_H__

#include <condition_variable>
#include <memory>
#include <mutex>
#include <utility>

namespace exercises::t_s_s {

class EmptyListQueue : public std::runtime_error {
 public:
  EmptyListQueue() : std::runtime_error("empty list queue") {}
};

template <typename T>
class ThreadSafeListQueue {
 public:
  ThreadSafeListQueue();

  ThreadSafeListQueue(const ThreadSafeListQueue<T> &other);

  // ThreadSafeListQueue(ThreadSafeListQueue<T> &&other) noexcept = delete;

  // ThreadSafeListQueue<T> &operator=(const ThreadSafeListQueue<T> &other) =
  //     delete;

  // ThreadSafeListQueue<T> &operator=(ThreadSafeListQueue<T> &&other) noexcept
  // =
  //     delete;

  ~ThreadSafeListQueue() = default;

  bool empty() const;

  void push(T value);

  bool tryPop(T &value);

  std::unique_ptr<T> tryPop();

  std::unique_ptr<T> waitAndPop();

 private:
  struct Node {
    std::unique_ptr<T> _data{};
    std::unique_ptr<Node> _next{};

    const auto &next() const { return _next; }
    auto &next() { return _next; }
    const auto &data() const { return _data; }
    auto &data() { return _data; }
  };

  auto getTail() const -> Node * {
    std::scoped_lock lock{_tail_m};
    return _tail;
  }

  mutable std::mutex _tail_m, _head_m;
  std::condition_variable _head_cv;

  std::unique_ptr<Node> _head;
  Node *_tail;
};

template <typename T>
ThreadSafeListQueue<T>::ThreadSafeListQueue()
    : _head{std::make_unique<Node>()}, _tail{_head.get()} {}

template <typename T>
bool ThreadSafeListQueue<T>::empty() const {
  std::scoped_lock lock{_head_m};
  return _head.get() == getTail();
}

template <typename T>
void ThreadSafeListQueue<T>::push(T value) {
  auto data = std::make_unique<T>(value);
  auto p = std::make_unique<Node>();

  {
    std::scoped_lock lock{_tail_m};
    _tail->data() = std::move(data);
    auto const new_tail = p.get();
    _tail->next() = std::move(p);
    _tail = new_tail;
  }

  _head_cv.notify_one();
}

template <typename T>
bool ThreadSafeListQueue<T>::tryPop(T &value) {
  std::unique_lock h_lock{_head_m};
  if (_head.get() == getTail()) {
    return false;
  }

  auto const old_head = std::move(_head);
  _head = std::move(old_head->next());
  h_lock.unlock();

  value = std::move(*old_head->data());
  return true;
}

template <typename T>
std::unique_ptr<T> ThreadSafeListQueue<T>::tryPop() {
  std::unique_lock h_lock{_head_m};
  if (_head.get() == getTail()) {
    return {};
  }

  auto old_head = std::move(_head);
  _head = std::move(old_head->next());
  h_lock.unlock();

  auto &&res = std::move(old_head->data());
  return res;
}

template <typename T>
std::unique_ptr<T> ThreadSafeListQueue<T>::waitAndPop() {
  std::unique_lock h_lock{_head_m};
  _head_cv.wait(h_lock, [this] { return _head.get() != getTail(); });

  auto old_head = std::move(_head);
  _head = std::move(old_head->next());
  h_lock.unlock();

  auto &&res = std::move(old_head->data());
  return res;
}

}  // namespace exercises::t_s_s

#endif  // __ConcurrencyAction_THREAD_SAFE_LIST_QUEUE_H__
