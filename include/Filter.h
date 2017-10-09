#pragma once

#include <string>
#include <vector>

#include <llvm/IR/Function.h>

namespace mull {

class Filter {
public:
  bool shouldSkipFunction(llvm::Function *function);

  void skipByName(std::string &nameSubstring);
  void skipByName(const char *nameSubstring);

  void skipByLocation(std::string &locationSubstring);
  void skipByLocation(const char *locationSubstring);
private:
  std::vector<std::string> names;
  std::vector<std::string> locations;
};

}
