#ifndef __ATM_CSP_STATE_H__
#define __ATM_CSP_STATE_H__

#include <memory>

class State {
 public:
  State() = default;

  State(const State &) = default;

  State(State &&) noexcept = default;

  State &operator=(const State &) = default;

  State &operator=(State &&) noexcept = default;

  virtual ~State() = default;

  virtual std::unique_ptr<State> action() = 0;

 private:
};

#endif  // __ATM_CSP_STATE_H__
