#ifndef __QTM_CSP_BANK_STATE_H__
#define __QTM_CSP_BANK_STATE_H__

#include <memory>
#include <utility>

#include "bank/bank.h"
#include "msg_fsm/msg_state.h"

class BankState : public MsgState {
 public:
  BankState(Bank* bank, std::unique_ptr<Receiver>& incomer);

  BankState(BankState&& rhs) noexcept = default;

  ~BankState() override = default;

 public:
  template <typename Msg>
  void sendToATM(Msg&& msg);

  Bank* bank() const;

 private:
  Bank* _bank{nullptr};
};

class BankStateWaiting : public BankState {
 public:
  BankStateWaiting(Bank* bank, std::unique_ptr<Receiver>& incomer);

  BankStateWaiting(BankStateWaiting&& rhs) noexcept = default;

  ~BankStateWaiting() override = default;

  std::unique_ptr<State> action() override;

 private:
};

template <typename Msg>
void BankState::sendToATM(Msg&& msg) {
  if (!_bank) {
    return;
  }

  _bank->sendToAtm(std::forward<Msg>(msg));
}

#endif  // __QTM_CSP_BANK_STATE_H__
