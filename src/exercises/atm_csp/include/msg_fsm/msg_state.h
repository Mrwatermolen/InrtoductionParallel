#ifndef __ATM_CSP_MSG_STATE_H__
#define __ATM_CSP_MSG_STATE_H__

#include <memory>

#include "dispatcher/dispatcher.h"
#include "dispatcher/receiver.h"
#include "fsm/state.h"

class MsgState : public State {
 public:
  explicit MsgState(std::unique_ptr<Receiver>& incomer);

  ~MsgState() override = default;

 protected:
  DispatcherHead wait();

  std::unique_ptr<Receiver>& incomer();

 private:
  std::unique_ptr<Receiver>& _incomer;
};

inline MsgState::MsgState(std::unique_ptr<Receiver>& incomer)
    : _incomer{incomer} {}

inline DispatcherHead MsgState::wait() { return _incomer->wait(); }

inline std::unique_ptr<Receiver>& MsgState::incomer() { return _incomer; }

#endif  // __ATM_CSP_MSG_STATE_H__
