#ifndef __ATM_CSP_MSG_MACHINE_H__
#define __ATM_CSP_MSG_MACHINE_H__

#include <iostream>
#include <memory>

#include "dispatcher/receiver.h"
#include "fsm/machine.h"
#include "fsm/state.h"

class MsgMachine : public Machine {
 public:
  // explicit MsgMachine(std::unique_ptr<Receiver> incomer);
  
  MsgMachine() = default;

  MsgMachine(const MsgMachine &) = delete;

  MsgMachine(MsgMachine &&) noexcept = default;

  MsgMachine &operator=(const MsgMachine &) = delete;

  MsgMachine &operator=(MsgMachine &&) noexcept = default;

  ~MsgMachine() override = default;

  void run() override;

  std::unique_ptr<Sender> sender() const;

 protected:
  const std::unique_ptr<Receiver> &incomer() const { return _incomer; }

  std::unique_ptr<Receiver> &incomer() { return _incomer; }

 private:
  std::unique_ptr<Receiver> _incomer{std::make_unique<Receiver>()};

  std::unique_ptr<State> _state{};
};

// MsgMachine::MsgMachine(std::unique_ptr<Receiver> incomer)
//     : _incomer(std::move(incomer)) {}

inline void MsgMachine::run() {
  try {
    Machine::run();
  } catch (const CloseQueueMsg &e) {
    std::cout << "MsgMachine is closed\n";
  }
}

inline std::unique_ptr<Sender> MsgMachine::sender() const {
  return incomer()->getSender();
}

#endif  // __ATM_CSP_MSG_MACHINE_H__
