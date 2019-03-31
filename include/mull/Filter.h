#pragma once

#include <string>
#include <vector>

#include <llvm/IR/Function.h>
#include <llvm/IR/Instruction.h>

namespace mull {

class Filter {
public:
  bool shouldSkipFunction(llvm::Function *function);
  bool shouldSkipInstruction(llvm::Instruction *instruction);
  bool shouldSkipTest(const std::string &testName);

  void skipByName(const std::string &nameSubstring);
  void skipByName(const char *nameSubstring);

  void skipByLocation(const std::string &locationSubstring);
  void skipByLocation(const char *locationSubstring);

  void includeTest(const std::string &testName);
  void includeTest(const char *testName);

private:
  std::vector<std::string> names;
  std::vector<std::string> locations;
  std::vector<std::string> tests;
};

} // namespace mull
