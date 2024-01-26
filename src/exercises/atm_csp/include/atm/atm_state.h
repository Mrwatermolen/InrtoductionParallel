#ifndef __ATM_CSP_ATM_STATE_H__
#define __ATM_CSP_ATM_STATE_H__

#include <cstddef>
#include <memory>
#include <utility>

#include "atm/atm.h"
#include "fsm/state.h"
#include "msg_fsm/msg_state.h"

class ATMState : public MsgState {
 public:
  explicit ATMState(ATM* atm, std::unique_ptr<Receiver>& incomer);

  ATMState(const ATMState&) = delete;

  ATMState(ATMState&&) noexcept = default;

  ATMState& operator=(const ATMState&) = delete;

  ATMState& operator=(ATMState&&) noexcept = delete;

  ~ATMState() override = default;

 protected:
  template <typename Msg>
  void sendToScreen(Msg&& msg);

  template <typename Msg>
  void sendtoBank(Msg&& msg);

  ATM* atm() const;

  std::size_t token() const;

  bool blocking() const;

  void setBlocking(bool blocking);

  std::size_t lastedOpId() const;

  std::size_t increaseLastedOpId();

  bool msgMatched(std::size_t msg_token, std::size_t msg_op_id) const;

 private:
  ATM* _atm;

  std::size_t _token{0};
  bool _blocking{false};
  std::size_t _lasted_op_id{0};
};

template <typename Msg>
inline void ATMState::sendToScreen(Msg&& msg) {
  if (!_atm) {
    return;
  }

  _atm->sendToScreen(std::forward<Msg>(msg));
}

template <typename Msg>
inline void ATMState::sendtoBank(Msg&& msg) {
  if (!_atm) {
    return;
  }

  _atm->sendToBank(std::forward<Msg>(msg));
}

class ATMStateIdle : public ATMState {
 public:
  ATMStateIdle(ATM* atm, std::unique_ptr<Receiver>& incomer);

  ATMStateIdle(ATMStateIdle&&) noexcept = default;

  ~ATMStateIdle() override = default;

  std::unique_ptr<State> action() override;

 private:
};

class ATMStateWaitPin : public ATMState {
 public:
  explicit ATMStateWaitPin(ATM* atm, std::unique_ptr<Receiver>& incomer,
                           std::string _card_number, std::string pin = "");

  ATMStateWaitPin(ATMStateWaitPin&&) noexcept = default;

  ~ATMStateWaitPin() override = default;

  std::unique_ptr<State> action() override;

 private:
  std::string _card_number;
  std::string _current_pin;
  unsigned int _pin_tries{3};

  void validPin(const std::string& pin);
};

class ATMStateOPerationAccount : public ATMState {
 public:
  ATMStateOPerationAccount(ATM* atm, std::unique_ptr<Receiver>& incomer,
                           std::string card_number);

  ATMStateOPerationAccount(ATMStateOPerationAccount&&) noexcept = default;

  ~ATMStateOPerationAccount() override = default;

  std::unique_ptr<State> action() override;

 private:
  std::string _card_number;

  bool withdraw(const std::string& amount);

  bool deposit(const std::string& amount);

  std::string balance() const;
};

#endif  // __ATM_CSP_ATM_STATE_H__
