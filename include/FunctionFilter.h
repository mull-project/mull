#pragma once

#include <string>
#include <vector>

#include <llvm/IR/Function.h>

namespace mull {

class FunctionFilter {
public:
  virtual bool shouldSkipFunction(llvm::Function *function) { return false; };
  virtual ~FunctionFilter() {}
};

class StaticFunctionFilter : public FunctionFilter {
public:
  StaticFunctionFilter(std::vector<std::string> names) : namesToSkip(names) {}

  bool shouldSkipFunction(llvm::Function *function) {
    auto result = std::find(namesToSkip.begin(), namesToSkip.end(), function->getName());
    return result != namesToSkip.end();
  }

private:
  std::vector<std::string> namesToSkip;
};
}

