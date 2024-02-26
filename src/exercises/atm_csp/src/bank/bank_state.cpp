#include "bank/bank_state.h"

#include <memory>
#include <utility>

#include "atm/atm_handle_msg.h"
#include "bank/bank.h"
#include "bank/bank_handle_msg.h"
#include "dispatcher/receiver.h"
#include "fsm/state.h"
#include "msg_fsm/msg_state.h"

BankState::BankState(Bank* bank, std::unique_ptr<Receiver>& incomer)
    : MsgState{incomer}, _bank{bank} {}

Bank* BankState::bank() const { return _bank; }

BankStateWaiting::BankStateWaiting(Bank* bank,
                                   std::unique_ptr<Receiver>& incomer)
    : BankState{bank, incomer} {}

std::unique_ptr<State> BankStateWaiting::action() {
  std::unique_ptr<BankState> next_state{nullptr};

  wait().dispatch<BankHandleMsgValidPin>([&next_state, this](const auto& msg) {
    std::cout << "Bank: get msg from ATM\n";
    ATMHandleMsgOp send_msg = ATMHandleMsgOp{msg._token, msg._id, false};
    if (msg._pin == "1234") {
      send_msg._is_success = true;
      std::cout << "Bank: Valid pass\n";
    }

    // sendToATM(ATMHandleMsgOp{msg._token, msg._id, false}); //ok
    // sendToATM((send_msg)); //err
    sendToATM(std::move(send_msg));
    next_state = std::make_unique<BankStateWaiting>(std::move(*this));
  });

  return next_state;
}