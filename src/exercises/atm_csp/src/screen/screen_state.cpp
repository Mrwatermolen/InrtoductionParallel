#include "screen/screen_state.h"

#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <utility>

#include "screen/screen_handle_msg.h"

ScreenState::ScreenState(std::unique_ptr<Receiver>& incomer)
    : MsgState{incomer} {}

ScreenStateWaiting::ScreenStateWaiting(std::unique_ptr<Receiver>& incomer)
    : ScreenState{incomer} {}

std::unique_ptr<State> ScreenStateWaiting::action() {
  std::unique_ptr<ScreenState> next_state{nullptr};

  wait()
      .dispatch<ScreenHandleMsgShowWaitingForCard>(
          [&next_state, this](
              [[maybe_unused]] const ScreenHandleMsgShowWaitingForCard& msg) {
            std::cout << "Screen: Please insert card(i)\n";
            next_state = std::make_unique<ScreenStateWaiting>(std::move(*this));
          })
      .dispatch<ScreenHandleMsgShowAcquiringPin>(
          [&next_state, this](const ScreenHandleMsgShowAcquiringPin& msg) {
            std::stringstream ss;
            ss << "Screen: Please enter pin(0-9) for card " << msg._card_number
               << "\n";
            std::cout << ss.str();
            next_state = std::make_unique<ScreenStateWaiting>(std::move(*this));
          })
      .dispatch<ScreenHandleMsgShowWaitingValidPin>(
          [&next_state, this]([[maybe_unused]] const auto& msg) {
            std::cout << "Screen: Please wait for valid pin\n";
            next_state = std::make_unique<ScreenStateWaiting>(std::move(*this));
          })
      .dispatch<ScreenHandleMsgShowInvalidPin>(
          [&next_state, this](const ScreenHandleMsgShowInvalidPin& msg) {
            std::stringstream ss;
            ss << "Screen: Invalid pin for card " << msg._card_number << "\n";
            ss << "Remaining tries: " << msg._tries_left << "\n";
            ss << "Please enter pin(0-9) for card " << msg._card_number << "\n";
            std::cout << ss.str();
            next_state = std::make_unique<ScreenStateWaiting>(std::move(*this));
          })
      .dispatch<ScreenHandleMsgShowCardBlocked>([&](const auto& msg) {
        std::stringstream ss;
        ss << "Screen: Card " << msg._card_number << " is blocked\n";
        ss << "Screen: Please eject card\n";
        std::cout << ss.str();
        next_state = std::make_unique<ScreenStateWaiting>(std::move(*this));
      })
      .dispatch<ScreenHandleMsgShowOperationMenu>(
          [&](const ScreenHandleMsgShowOperationMenu& msg) {
            std::stringstream ss;
            ss << "Screen: Welcome to ATM\n";
            ss << "Screen: Please choose operation for card "
               << msg._card_number << "\n";
            ss << "1. Withdraw\n";
            ss << "2. Deposit\n";
            ss << "3. Balance\n";
            std::cout << ss.str();
            next_state = std::make_unique<ScreenStateAccountOperation>(
                incomer(), msg._card_number);
          })
      .dispatch<ScreenHandleMsgShowEjectCard>([&](const auto& msg) {
        std::stringstream ss;
        ss << "Screen: Please take your card " << msg._card_number << "\n";
        ss << "Screen: Goodbye\n";
        std::cout << ss.str();
        next_state = std::make_unique<ScreenStateWaiting>(std::move(*this));
      });

  return next_state;
}

ScreenStateAccountOperation::ScreenStateAccountOperation(
    std::unique_ptr<Receiver>& incomer, std::string card_number)
    : ScreenState{incomer}, _card_number{std::move(card_number)} {}

std::unique_ptr<State> ScreenStateAccountOperation::action() {
  std::unique_ptr<ScreenState> next_state{nullptr};

  wait()
      .dispatch<ScreenHandleMsgShowEjectCard>([&](const auto& msg) {
        std::stringstream ss;
        ss << "Screen: Please take your card " << msg._card_number << "\n";
        ss << "Screen: Goodbye\n";
        std::cout << ss.str();
        next_state = std::make_unique<ScreenStateWaiting>(incomer());
      })
      .dispatch<ScreenHandleMsgShowWithdraw>([&](const auto& msg) {
        std::stringstream ss;
        if (msg._is_success) {
          ss << "Screen: Withdraw " << msg._amount << " from card "
             << msg._card_number << "\n";
        } else {
          ss << "Screen: Withdraw failed\n";
        }
        std::cout << ss.str();
        next_state =
            std::make_unique<ScreenStateAccountOperation>(std::move(*this));
      })
      .dispatch<ScreenHandleMsgShowDeposit>([&](const auto& msg) {
        std::stringstream ss;
        if (msg._is_success) {
          ss << "Screen: Deposit " << msg._amount << " to card "
             << msg._card_number << "\n";
        } else {
          ss << "Screen: Deposit failed\n";
        }
        std::cout << ss.str();
        next_state =
            std::make_unique<ScreenStateAccountOperation>(std::move(*this));
      })
      .dispatch<ScreenHandleMsgShowBalance>([&](const auto& msg) {
        std::stringstream ss;
        ss << "Screen: Balance of card " << msg._card_number << " is "
           << msg._amount << "\n";
        std::cout << ss.str();
        next_state =
            std::make_unique<ScreenStateAccountOperation>(std::move(*this));
      });

  return nullptr;
}
