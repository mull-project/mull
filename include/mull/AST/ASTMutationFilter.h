#pragma once

#include "mull/Diagnostics/Diagnostics.h"
#include "mull/Filters/MutationFilter.h"
#include "mull/JunkDetection/CXX/ASTStorage.h"

namespace mull {

class ASTMutationFilter: public MutationFilter {
public:
  ASTMutationFilter(Diagnostics &diagnostics, ASTStorage &astStorage);
  bool shouldSkip(MutationPoint *point) override ;
  std::string name() override;

private:
  Diagnostics &diagnostics;
  ASTStorage &astStorage;
};
}