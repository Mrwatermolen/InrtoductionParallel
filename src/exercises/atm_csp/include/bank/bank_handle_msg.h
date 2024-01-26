#include <cstddef>
#include <string>

struct BankHandleMsgBase {
  std::size_t _token;
  std::size_t _id;
};

struct BankHandleMsgValidPin : public BankHandleMsgBase {
  std::string _card_number;
  std::string _pin;
};

struct BankHandleMsgAccountOp : public BankHandleMsgBase {
  enum class Type { Deposit, Withdraw, Balance };
  Type _type;
  std::string _card_number;
};
