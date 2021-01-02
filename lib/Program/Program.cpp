#include "mull/Program/Program.h"

#include <cassert>
#include <llvm/IR/Constants.h>
#include <utility>

using namespace mull;

Program::Program(std::vector<std::unique_ptr<Bitcode>> bitcode) {
  for (auto &bc : bitcode) {
    _bitcode.emplace_back(std::move(bc));
  }
}

std::vector<std::unique_ptr<Bitcode>> &Program::bitcode() {
  return _bitcode;
}
