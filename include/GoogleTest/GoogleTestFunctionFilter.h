#pragma once

#include <FunctionFilter.h>

namespace mull {
class GoogleTestFunctionFilter : public FunctionFilter {
public:
  bool shouldSkipFunction(llvm::Function *function) override;
};
}


