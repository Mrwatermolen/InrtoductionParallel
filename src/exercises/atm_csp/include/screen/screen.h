#ifndef __ATM_CSP_SCREEN_H__
#define __ATM_CSP_SCREEN_H__

#include "msg_fsm/msg_machine.h"

class Screen : public MsgMachine {
 public:
  Screen() = default;

  Screen(const Screen&) = delete;

  Screen(Screen&&) = default;

  Screen& operator=(const Screen&) = delete;

  Screen& operator=(Screen&&) = delete;

  ~Screen() override = default;

 private:
  void init() override;
};

#endif  // __ATM_CSP_SCREEN_H__
