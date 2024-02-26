#ifndef __ATM_CSP_MACHINE_H__
#define __ATM_CSP_MACHINE_H__

#include <memory>

#include "fsm/state.h"

class Machine {
 public:
  Machine() = default;

  Machine(const Machine &) = delete;

  Machine(Machine &&) noexcept = default;

  Machine &operator=(const Machine &) = delete;

  Machine &operator=(Machine &&) noexcept = default;

  virtual ~Machine() = default;

  virtual void run();

 protected:
  virtual void init() = 0;

  void transition(std::unique_ptr<State> state);

 private:
  std::unique_ptr<State> _state{};
};

inline void Machine::transition(std::unique_ptr<State> state) {
  _state = std::move(state);
}

inline void Machine::run() {
  init();

  while (_state) {
    transition(_state->action());
  }
}

#endif  // __ATM_CSP_MACHINE_H__
