#ifndef __ATM_CSP_MESSAGE_H__
#define __ATM_CSP_MESSAGE_H__

#include "dispatcher/thread_safe_queue.h"

class Message {
 public:
  Message() = default;

  Message(const Message &) = default;

  Message(Message &&) noexcept = default;

  Message &operator=(const Message &) = default;

  Message &operator=(Message &&) noexcept = default;

  virtual ~Message() noexcept = default;
};

template <typename Msg>
class MessageWrapper : public Message {
 public:
  explicit MessageWrapper(Msg msg) : _msg{msg} {}

  // explicit MessageWrapper(Msg &&msg) : _msg(msg) {}

  MessageWrapper(const MessageWrapper &) = default;

  MessageWrapper(MessageWrapper &&) noexcept = default;

  MessageWrapper &operator=(const MessageWrapper &) = default;

  MessageWrapper &operator=(MessageWrapper &&) noexcept = default;

  ~MessageWrapper() noexcept override = default;

  const Msg &content() const { return _msg; }

  Msg &content() { return _msg; }

 private:
  Msg _msg;
};

using MessageQueue = ThreadSafeQueue<std::shared_ptr<Message>>;

#endif  // __ATM_CSP_MESSAGE_H__
