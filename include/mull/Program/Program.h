#pragma once

#include "mull/Bitcode.h"

#include <llvm/IR/Function.h>
#include <llvm/IR/Module.h>
#include <llvm/Object/ObjectFile.h>

#include <vector>

namespace mull {

class Program {
public:
  explicit Program(std::vector<std::unique_ptr<Bitcode>> bitcode);
  std::vector<std::unique_ptr<Bitcode>> &bitcode();

private:
  std::vector<std::unique_ptr<Bitcode>> _bitcode;
};

} // namespace mull
