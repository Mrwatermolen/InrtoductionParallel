#include <thread>

#include "atm/atm.h"
#include "atm/atm_handle_msg.h"
#include "bank/bank.h"
#include "screen/screen.h"

int main() {
  ATM atm{};
  Bank bank{};
  Screen screen{};

  atm.setScreen(screen.sender());
  atm.setBank(bank.sender());

  bank.setAtm(atm.sender());

  struct JThreadGuard {
    ~JThreadGuard() {
      if (_t.joinable()) {
        _t.join();
      }
    }
    std::thread _t;
  };

  auto atm_run = JThreadGuard{std::thread{&ATM::run, &atm}};
  auto bank_run = JThreadGuard{std::thread{&Bank::run, &bank}};
  auto screen_run = JThreadGuard{std::thread{&Screen::run, &screen}};

  char c;

  auto key_input_notifier = atm.sender();

  while (true) {
    std::cin >> c;

    if (c == 'i') {
      key_input_notifier->send(ATMHandleMsgInsertedCard{"214214"});
      continue;
    }

    if (c == 'a') {
      key_input_notifier->send(ATMHandleMsgKeyPressedEnter{});
      continue;
    }

    if (c == 'c') {
      key_input_notifier->send(ATMHandleMsgKeyPressedCancel{});
      continue;
    }

    if (c == 'b') {
      key_input_notifier->send(ATMHandleMsgKeyPressedBackspace{});
      continue;
    }

    if (c == 'q') {
      key_input_notifier->send(ATMHandleMsgKeyPressedEject{});
      continue;
    }

    if ('0' <= c && c <= '9') {
      key_input_notifier->send(ATMHandleMsgKeyPressedDigit{c});
      continue;
    }
  }

  return 0;
}