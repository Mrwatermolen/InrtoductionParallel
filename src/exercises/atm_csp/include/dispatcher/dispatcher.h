#ifndef __ATM_CSP_DISPATCHER_H__
#define __ATM_CSP_DISPATCHER_H__

#include <exception>
#include <memory>
#include <utility>

#include "dispatcher/message.h"

struct CloseQueueMsg {};

class DispatcherException : public std::exception {};

class DispatcherExceptionGetCloseQueueMsg : public DispatcherException {
 public:
  const char *what() const noexcept override {
    return "Dispatcher get close queue msg";
  }
};

template <typename Msg, typename PrevNode, typename Func>
class DispatcherNode;

class DispatcherHead {
  template <typename Msg, typename PrevNodePtr, typename Func>
  friend class DispatcherNode;

 public:
  explicit DispatcherHead(std::shared_ptr<MessageQueue> q);

  DispatcherHead(const DispatcherHead &) = delete;

  DispatcherHead(DispatcherHead &&) noexcept = default;

  DispatcherHead &operator=(const DispatcherHead &) = delete;

  DispatcherHead &operator=(DispatcherHead &&) noexcept = default;

  ~DispatcherHead() noexcept(false);

  template <typename OtherMsg, typename OtherFunc>
  DispatcherNode<OtherMsg, DispatcherHead, OtherFunc> dispatch(
      OtherFunc &&func);

 private:
  bool _tail{true};
  std::shared_ptr<MessageQueue> _q{};

  void join();

  void waitMsg();

  bool tryHandleMsg(const std::shared_ptr<Message> &msg);
};

template <typename Msg, typename PrevNode, typename Func>
class DispatcherNode {
  template <typename OtherMsg, typename OtherPrevNode, typename OtherFunc>
  friend class DispatcherNode;

 public:
  using NodePtr = PrevNode *;

  DispatcherNode(NodePtr prev, Func func, std::shared_ptr<MessageQueue> q);

  DispatcherNode(const DispatcherNode &) = delete;

  DispatcherNode(DispatcherNode &&) noexcept = delete;

  DispatcherNode &operator=(const DispatcherNode &) = delete;

  DispatcherNode &operator=(DispatcherNode &&) noexcept = delete;

  ~DispatcherNode();

  template <typename OtherMsg, typename OtherFunc>
  DispatcherNode<OtherMsg, DispatcherNode, OtherFunc> dispatch(
      OtherFunc &&func);

 private:
  bool _tail{true};
  NodePtr _prev;
  Func _func;
  std::shared_ptr<MessageQueue> _q{};

  void join();

  void waitMsg();

  bool tryHandleMsg(const std::shared_ptr<Message> &msg);
};

inline DispatcherHead::DispatcherHead(std::shared_ptr<MessageQueue> q)
    : _q{std::move(q)} {}

inline DispatcherHead::~DispatcherHead() noexcept(false) {
  if (_tail) {
    waitMsg();
  }
}

template <typename OtherMsg, typename OtherFunc>
inline DispatcherNode<OtherMsg, DispatcherHead, OtherFunc>
DispatcherHead::dispatch(OtherFunc &&func) {
  return DispatcherNode<OtherMsg, DispatcherHead, OtherFunc>{
      this, std::forward<OtherFunc>(func), _q};
}

inline void DispatcherHead::join() { _tail = false; }

inline void DispatcherHead::waitMsg() {
  while (_q) {
    std::shared_ptr<Message> msg;
    _q->waitAndPop(msg);
    if (tryHandleMsg(msg)) {
      break;
    }
  }
}

inline bool DispatcherHead::tryHandleMsg(const std::shared_ptr<Message> &msg) {
  auto close_msg =
      std::dynamic_pointer_cast<MessageWrapper<CloseQueueMsg>>(msg);
  if (close_msg == nullptr) {
    return false;
  }

  throw DispatcherExceptionGetCloseQueueMsg{};
}

template <typename Msg, typename PrevNode, typename Func>
inline DispatcherNode<Msg, PrevNode, Func>::DispatcherNode(
    NodePtr prev, Func func, std::shared_ptr<MessageQueue> q)
    : _prev{prev}, _func{std::move(func)}, _q{std::move(q)} {
  prev->join();
}

template <typename Msg, typename PrevNode, typename Func>
inline DispatcherNode<Msg, PrevNode, Func>::~DispatcherNode() {
  if (_tail) {
    waitMsg();
  }
}

template <typename Msg, typename PrevNode, typename Func>
template <typename OtherMsg, typename OtherFunc>
inline DispatcherNode<OtherMsg, DispatcherNode<Msg, PrevNode, Func>, OtherFunc>
DispatcherNode<Msg, PrevNode, Func>::dispatch(OtherFunc &&func) {
  return DispatcherNode<OtherMsg, DispatcherNode, OtherFunc>{
      this, std::forward<OtherFunc>(func), _q};
}

template <typename Msg, typename PrevNode, typename Func>
inline void DispatcherNode<Msg, PrevNode, Func>::join() {
  _tail = false;
}

template <typename Msg, typename PrevNode, typename Func>
inline void DispatcherNode<Msg, PrevNode, Func>::waitMsg() {
  while (_q) {
    std::shared_ptr<Message> msg;
    _q->waitAndPop(msg);
    if (tryHandleMsg(msg)) {
      break;
    }
  }
}

template <typename Msg, typename PrevNode, typename Func>
inline bool DispatcherNode<Msg, PrevNode, Func>::tryHandleMsg(
    const std::shared_ptr<Message> &msg) {
  auto my_msg_wrapper = std::dynamic_pointer_cast<MessageWrapper<Msg>>(msg);
  if (my_msg_wrapper != nullptr) {
    _func(my_msg_wrapper->content());
    return true;
  }

  return _prev->tryHandleMsg(msg);
}

#endif  // __ATM_CSP_DISPATCHER_H__
