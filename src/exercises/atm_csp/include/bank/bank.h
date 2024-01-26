#ifndef __ATM_CSP_BANK_H__
#define __ATM_CSP_BANK_H__

#include <memory>

#include "dispatcher/receiver.h"
#include "msg_fsm/msg_machine.h"

class Bank : public MsgMachine {
  friend class BankState;

 public:
  Bank() = default;

  Bank(const Bank&) = delete;

  Bank& operator=(const Bank&) = delete;

  Bank(Bank&&) = default;

  Bank& operator=(Bank&&) = delete;

  ~Bank() override = default;

  void setAtm(std::unique_ptr<Sender> atm);

 private:
  std::unique_ptr<Sender> _atm;

  void init() override;

  template <typename Msg>
  void sendToAtm(Msg&& msg);
};

template <typename Msg>
inline void Bank::sendToAtm(Msg&& msg) {
  if (!_atm) {
    return;
  }
  _atm->send(std::forward<Msg>(msg));
}

#endif  // __ATM_CSP_BANK_H__
