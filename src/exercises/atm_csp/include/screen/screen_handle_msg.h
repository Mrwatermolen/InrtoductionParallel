#ifndef __ATM_CSP_SCREEN_HANDLE_MSG_H__
#define __ATM_CSP_SCREEN_HANDLE_MSG_H__

#include <string>

struct ScreenHandleMsgShowWaitingForCard {};

struct ScreenHandleMsgShowAcquiringPin {
  std::string _card_number;
};

struct ScreenHandleMsgShowWaitingValidPin {};

struct ScreenHandleMsgShowInvalidPin {
  std::string _card_number;
  unsigned int _tries_left;
};

struct ScreenHandleMsgShowCardBlocked {
  std::string _card_number;
};

struct ScreenHandleMsgShowOperationMenu {
  std::string _card_number;
};

struct ScreenHandleMsgShowEjectCard {
  std::string _card_number;
};

struct ScreenHandleMsgShowWaitingAccountOp {
  std::string _card_number;
  std::string _text;
};

struct ScreenHandleMsgShowWithdraw {
  std::string _card_number;
  bool _is_success;
  std::string _amount;
};

struct ScreenHandleMsgShowDeposit {
  std::string _card_number;
  bool _is_success;
  std::string _amount;
};

struct ScreenHandleMsgShowBalance {
  std::string _card_number;
  std::string _amount;
};

#endif  // __ATM_CSP_SCREEN_HANDLE_MSG_H__
