#pragma once

#include <cstdint>
#include <stack>

namespace mull {
struct InstrumentationInfo {
  InstrumentationInfo() :callTreeMapping(nullptr), callstack() {}
  uint32_t *callTreeMapping;
  std::stack<uint32_t> callstack;
};
}
