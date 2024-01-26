#include "atm/atm_state.h"

#include <cstddef>
#include <iostream>
#include <limits>
#include <memory>
#include <random>
#include <string>
#include <thread>
#include <utility>

#include "atm/atm.h"
#include "atm/atm_handle_msg.h"
#include "bank/bank_handle_msg.h"
#include "screen/screen_handle_msg.h"

static std::size_t getUuid() {
  static thread_local std::default_random_engine generator(
      std::random_device{}());
  std::uniform_int_distribution<std::size_t> distribution(
      1, std::numeric_limits<std::size_t>::max());
  return distribution(generator);
}

ATMState::ATMState(ATM* atm, std::unique_ptr<Receiver>& incomer)
    : MsgState{incomer}, _atm{atm}, _token{getUuid()} {}

ATM* ATMState::atm() const { return _atm; }

std::size_t ATMState::token() const { return _token; }

bool ATMState::blocking() const { return _blocking; }

void ATMState::setBlocking(bool blocking) { _blocking = blocking; }

std::size_t ATMState::lastedOpId() const { return _lasted_op_id; }

std::size_t ATMState::increaseLastedOpId() {
  _lasted_op_id++;
  return _lasted_op_id;
}

bool ATMState::msgMatched(std::size_t msg_token, std::size_t msg_op_id) const {
  return msg_token == token() && msg_op_id == lastedOpId();
}

ATMStateIdle::ATMStateIdle(ATM* atm, std::unique_ptr<Receiver>& incomer)
    : ATMState{atm, incomer} {
  setBlocking(false);
}

std::unique_ptr<State> ATMStateIdle::action() {
  std::unique_ptr<ATMState> next_state{};
  wait().dispatch<ATMHandleMsgInsertedCard>(
      [&next_state,
       this]([[maybe_unused]] const ATMHandleMsgInsertedCard& msg) {
        std::cout << (std::string("ATM: get card: ") + msg._card_number + "\n");
        sendToScreen(ScreenHandleMsgShowAcquiringPin{msg._card_number});
        next_state = std::make_unique<ATMStateWaitPin>(atm(), incomer(),
                                                       msg._card_number);
      });
  return next_state;
}

ATMStateWaitPin::ATMStateWaitPin(ATM* atm, std::unique_ptr<Receiver>& incomer,
                                 std::string card_number, std::string pin)
    : ATMState{atm, incomer},
      _card_number{std::move(card_number)},
      _current_pin{std::move(pin)} {}

std::unique_ptr<State> ATMStateWaitPin::action() {
  std::unique_ptr<ATMState> next_state{};

  wait()
      .dispatch<ATMHandleMsgKeyPressedDigit>([&](const auto& msg) {
        if (blocking()) {
          next_state = std::make_unique<ATMStateWaitPin>(std::move(*this));
          return;
        }

        if (_pin_tries == 0) {
          std::cout << "ATM: card is blocked\n";
          sendToScreen(ScreenHandleMsgShowCardBlocked{});
          next_state = std::make_unique<ATMStateWaitPin>(std::move(*this));
          return;
        }

        if (_current_pin.size() < 4) {
          _current_pin += msg._data;
        }

        next_state = std::make_unique<ATMStateWaitPin>(std::move(*this));
        return;
      })
      .dispatch<ATMHandleMsgKeyPressedBackspace>(
          [&]([[maybe_unused]] const auto& msg) {
            if (blocking()) {
              next_state = std::make_unique<ATMStateWaitPin>(std::move(*this));
              return;
            }

            if (_current_pin.empty()) {
              next_state = std::make_unique<ATMStateWaitPin>(std::move(*this));
              return;
            }

            _current_pin.pop_back();
            next_state = std::make_unique<ATMStateWaitPin>(std::move(*this));
            return;
          })
      .dispatch<ATMHandleMsgKeyPressedEnter>(
          [&]([[maybe_unused]] const auto& msg) {
            if (blocking()) {
              next_state = std::make_unique<ATMStateWaitPin>(std::move(*this));
              return;
            }

            if (_pin_tries == 0) {
              std::cout << "ATM: card is blocked\n";
              sendToScreen(ScreenHandleMsgShowCardBlocked{});
              next_state = std::make_unique<ATMStateWaitPin>(std::move(*this));
              return;
            }

            if (_current_pin.size() < 4) {
              std::cout << "ATM: pin is too small\n";
              sendToScreen(ScreenHandleMsgShowAcquiringPin{_card_number});
              next_state = std::make_unique<ATMStateWaitPin>(std::move(*this));
              return;
            }

            std::cout << "ATM: send valid request to bank\n";
            sendToScreen(ScreenHandleMsgShowWaitingValidPin{});
            validPin(_current_pin);
            next_state = std::make_unique<ATMStateWaitPin>(std::move(*this));
          })
      .dispatch<ATMHandleMsgKeyPressedCancel>(
          [&]([[maybe_unused]] const auto& msg) {
            sendToScreen(ScreenHandleMsgShowEjectCard{_card_number});
            next_state = std::make_unique<ATMStateIdle>(atm(), incomer());
            return;
          })
      .dispatch<ATMHandleMsgKeyPressedEject>(
          [&]([[maybe_unused]] const auto& msg) {
            std::cout << "ATM: Card Eject\n";
            sendToScreen(ScreenHandleMsgShowEjectCard{_card_number});
            next_state = std::make_unique<ATMStateIdle>(atm(), incomer());
            return;
          })
      .dispatch<ATMHandleMsgOp>([&](const auto& msg) {
        setBlocking(false);
        std::cout << "ATM: Get msg from bank\n";
        if (!msgMatched(msg._token, msg._op_id)) {
          next_state = std::make_unique<ATMStateWaitPin>(std::move(*this));
          return;
        }

        if (msg._is_success) {
          sendToScreen(ScreenHandleMsgShowOperationMenu{_card_number});
          next_state = std::make_unique<ATMStateOPerationAccount>(
              atm(), incomer(), _card_number);
          return;
        }

        --_pin_tries;
        if (_pin_tries == 0) {
          std::cout << "ATM: card is blocked\n";
          sendToScreen(ScreenHandleMsgShowCardBlocked{});
          next_state = std::make_unique<ATMStateWaitPin>(std::move(*this));
          return;
        }

        _current_pin = "";
        sendToScreen(ScreenHandleMsgShowInvalidPin{_card_number, _pin_tries});
        next_state = std::make_unique<ATMStateWaitPin>(std::move(*this));
        return;
      });

  return next_state;
}

void ATMStateWaitPin::validPin([[maybe_unused]] const std::string& pin) {
  setBlocking(true);
  auto op_id = increaseLastedOpId();
  sendtoBank(BankHandleMsgValidPin{BankHandleMsgBase{token(), op_id},
                                   _card_number, _current_pin});
}

ATMStateOPerationAccount::ATMStateOPerationAccount(
    ATM* atm, std::unique_ptr<Receiver>& incomer, std::string card_number)
    : ATMState{atm, incomer}, _card_number{std::move(card_number)} {}

std::unique_ptr<State> ATMStateOPerationAccount::action() {
  std::unique_ptr<ATMState> next_state =
      std::make_unique<ATMStateOPerationAccount>(atm(), incomer(),
                                                 _card_number);

  wait()
      .dispatch<ATMHandleMsgKeyPressedEject>(
          [&next_state, this]([[__maybe_unused__]] const auto& msg) {
            std::cout << "ATM: Card Eject\n";
            sendToScreen(ScreenHandleMsgShowEjectCard{_card_number});
            next_state = std::make_unique<ATMStateIdle>(atm(), incomer());
            return;
          })
      .dispatch<ATMHandleMsgKeyPressedDigit>(
          [&next_state, this](const auto& msg) {
            if (msg._data == "1") {
              withdraw("100");
              sendToScreen(ScreenHandleMsgShowWaitingAccountOp{
                  _card_number, "Waiting respond"});
              
            }
          });

  return next_state;
}

bool ATMStateOPerationAccount::withdraw(
    [[maybe_unused]] const std::string& amount) {
  [[maybe_unused]] std::size_t token = getUuid();
  std::this_thread::sleep_for(std::chrono::seconds(1));
  return true;
}

bool ATMStateOPerationAccount::deposit(
    [[maybe_unused]] const std::string& amount) {
  [[maybe_unused]] std::size_t token = getUuid();
  std::this_thread::sleep_for(std::chrono::seconds(1));
  return true;
}

std::string ATMStateOPerationAccount::balance() const {
  [[maybe_unused]] std::size_t token = getUuid();
  std::this_thread::sleep_for(std::chrono::seconds(1));
  return "1000";
}
