#ifndef __ATM_CSP_RECEIVER_H__
#define __ATM_CSP_RECEIVER_H__

#include <memory>
#include <utility>

#include "dispatcher/dispatcher.h"
#include "dispatcher/message.h"

class Sender {
 public:
  explicit Sender(std::shared_ptr<MessageQueue> _q);

  template <typename Msg>
  void send(Msg&& msg);

 private:
  std::shared_ptr<MessageQueue> _q{};
};

class Receiver {
 public:
  Receiver();

  explicit Receiver(std::shared_ptr<MessageQueue> q);

  DispatcherHead wait();

  std::unique_ptr<Sender> getSender() const;

 private:
  std::shared_ptr<MessageQueue> _q;
};

inline Sender::Sender(std::shared_ptr<MessageQueue> q) : _q{std::move(q)} {}

template <typename Msg>
inline void Sender::send(Msg&& msg) {
  if (!_q) {
    return;
  }
  _q->push(std::make_shared<MessageWrapper<Msg>>(std::forward<Msg>(msg)));
}

inline Receiver::Receiver() : _q{std::make_shared<MessageQueue>()} {}

inline Receiver::Receiver(std::shared_ptr<MessageQueue> q) : _q{std::move(q)} {}

inline DispatcherHead Receiver::wait() { return DispatcherHead{_q}; }

inline std::unique_ptr<Sender> Receiver::getSender() const {
  return std::make_unique<Sender>(_q);
}

#endif  // __ATM_CSP_RECEIVER_H__
