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
  Bitcode *bitcodeWithIdentifier(const std::string &identifier) const;

private:
  void addBitcode(std::unique_ptr<Bitcode> bitcode);

  std::vector<std::unique_ptr<Bitcode>> _bitcode;
  std::map<std::string, Bitcode *> bitcodeRegistry;
};

} // namespace mull
