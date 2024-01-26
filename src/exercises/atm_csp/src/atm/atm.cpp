#include "atm/atm.h"

#include <memory>
#include <utility>

#include "atm/atm_state.h"
#include "dispatcher/receiver.h"
#include "screen/screen_handle_msg.h"

// ATM::ATM(std::unique_ptr<Receiver> incomer)
//     : MsgMachine{std::move(incomer)} {
// }

void ATM::setBank(std::unique_ptr<Sender> bank) { _bank = std::move(bank); }

void ATM::setScreen(std::unique_ptr<Sender> screen) {
  _screen = std::move(screen);
}

void ATM::init() {
  sendToScreen<ScreenHandleMsgShowWaitingForCard>(ScreenHandleMsgShowWaitingForCard{});
  transition(std::make_unique<ATMStateIdle>(this, incomer()));
}
