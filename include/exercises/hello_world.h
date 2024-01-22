#ifndef __PROJECT_NAME_HELLO_WORLD_H__
#define __PROJECT_NAME_HELLO_WORLD_H__

#include <sstream>

namespace exercises::hello_world {

inline auto helloWorldString(int my_rank = 0) {
  std::stringstream ss;
  ss << "Hello World from process " << my_rank << "!";
  return ss.str();
}

void mpiPrintHelloWorld();

void ompPrintHelloWorld();

void threadPrintHelloWorld();

};  // namespace exercises::hello_world

#endif  // __PROJECT_NAME_HELLO_WORLD_H__
