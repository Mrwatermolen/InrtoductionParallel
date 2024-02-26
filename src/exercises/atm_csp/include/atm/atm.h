#ifndef __ATM_CSP_ATM_H__
#define __ATM_CSP_ATM_H__

#include <memory>
#include <utility>

#include "dispatcher/receiver.h"
#include "msg_fsm/msg_machine.h"

class ATM : public MsgMachine {
  friend class ATMState;

 public:
  ATM() = default;

  // explicit ATM(std::unique_ptr<Receiver> incomer);

  ATM(const ATM&) = delete;

  ATM(ATM&&) = default;

  ATM& operator=(const ATM&) = delete;

  ATM& operator=(ATM&&) = delete;

  ~ATM() override = default;

  void setBank(std::unique_ptr<Sender> bank);

  void setScreen(std::unique_ptr<Sender> screen);

 private:
  void init() override;

  template <typename Msg>
  void sendToScreen(Msg&& msg);

  template <typename Msg>
  void sendToBank(Msg&& msg);

  std::unique_ptr<Sender> _screen, _bank;
};

template <typename Msg>
inline void ATM::sendToScreen(Msg&& msg) {
  if (!_screen) {
    return;
  }
  _screen->send(std::forward<Msg>(msg));
}

template <typename Msg>
inline void ATM::sendToBank(Msg&& msg) {
  if (!_bank) {
    return;
  }

  _bank->send(std::forward<Msg>(msg));
}

#endif  // __ATM_CSP_ATM_H__
