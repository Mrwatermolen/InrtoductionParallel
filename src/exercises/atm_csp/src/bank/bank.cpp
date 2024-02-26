#include "bank/bank.h"

#include <memory>
#include <utility>

#include "bank/bank_state.h"
#include "dispatcher/receiver.h"

void Bank::setAtm(std::unique_ptr<Sender> atm) { _atm = std::move(atm); }

void Bank::init() {
  transition(std::make_unique<BankStateWaiting>(this, incomer()));
}
