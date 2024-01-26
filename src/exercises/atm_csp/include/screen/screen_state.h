#ifndef __ATM_CSP_SCREEN_STATE_H__
#define __ATM_CSP_SCREEN_STATE_H__

#include "msg_fsm/msg_state.h"

class ScreenState : public MsgState {
 public:
  explicit ScreenState(std::unique_ptr<Receiver>& incomer);
};

class ScreenStateWaiting : public ScreenState {
 public:
  explicit ScreenStateWaiting(std::unique_ptr<Receiver>& incomer);

  std::unique_ptr<State> action() override;
};

class ScreenStateAccountOperation : public ScreenState {
 public:
  ScreenStateAccountOperation(std::unique_ptr<Receiver>& incomer,
                              std::string _card_number);

  std::unique_ptr<State> action() override;

 private:
  std::string _card_number;
};

#endif  // __ATM_CSP_SCREEN_STATE_H__
