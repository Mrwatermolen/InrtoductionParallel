#ifndef __ATM_CSP_ATM_HANDLE_MSG_H__
#define __ATM_CSP_ATM_HANDLE_MSG_H__

#include <cstddef>
#include <string>

struct ATMHandleMsgInsertedCard {
  std::string _card_number;
};

struct ATMHandleMsgKeyPressed {
  enum class KeyType { Digit, Backspace, Enter, Cancel, Eject };
  KeyType _type;
  std::string _data;
};

struct ATMHandleMsgKeyPressedDigit : public ATMHandleMsgKeyPressed {
  explicit ATMHandleMsgKeyPressedDigit(char c) {
    _type = KeyType::Digit;
    _data = c;
  }
  std::string _data;
};

struct ATMHandleMsgKeyPressedBackspace : public ATMHandleMsgKeyPressed {
  ATMHandleMsgKeyPressedBackspace() { _type = KeyType::Backspace; }
};

struct ATMHandleMsgKeyPressedEnter : public ATMHandleMsgKeyPressed {
  ATMHandleMsgKeyPressedEnter() { _type = KeyType::Enter; }
};

struct ATMHandleMsgKeyPressedCancel : public ATMHandleMsgKeyPressed {
  ATMHandleMsgKeyPressedCancel() { _type = KeyType::Cancel; }
};

struct ATMHandleMsgKeyPressedEject : public ATMHandleMsgKeyPressed {
  ATMHandleMsgKeyPressedEject() { _type = KeyType::Eject; }
};

struct ATMHandleMsgOp {
  std::size_t _token;
  std::size_t _op_id;
  bool _is_success;
};

struct ATMHandleMsgOpWithdraw : public ATMHandleMsgOp {
  std::string _card_number;
  std::string _amount;
};

struct ATMHandleMsgOpDeposit : public ATMHandleMsgOp {
  std::string _card_number;
  std::string _amount;
};

struct ATMHandleMsgOpBalance : public ATMHandleMsgOp {
  std::string _card_number;
};

#endif  // __ATM_CSP_ATM_HANDLE_MSG_H__
