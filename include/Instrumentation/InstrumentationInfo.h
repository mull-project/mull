#pragma once

#include <cstdint>
#include <stack>

namespace mull {
struct InstrumentationInfo {
  InstrumentationInfo() :callTreeMapping(nullptr), callstack() {}
  uint64_t *callTreeMapping;
  std::stack<uint64_t> callstack;
};
}
