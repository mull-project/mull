#pragma once

#include "LLVMCompatibility.h"
#include "Mangler.h"

#include <utility>
#include <vector>

namespace mull {

/// This class is heavily inspired by the LLVM's ORCv1 LocalCXXRuntimeOverrides

class CXXRuntimeOverrides {
public:
  explicit CXXRuntimeOverrides(mull::Mangler &mangler);
  llvm_compat::JITSymbolInfo searchOverrides(const std::string &Name);
  void runDestructors();

private:
  using DestructorPtr = void (*)(void *);
  using CXXDestructorDataPair = std::pair<DestructorPtr, void *>;
  using CXXDestructorDataPairList = std::vector<CXXDestructorDataPair>;

  void addOverride(const std::string &name,
                   llvm_compat::JITTargetAddress address);
  static int CXAAtExitOverride(DestructorPtr destructor, void *arg,
                               void *DSOHandle);

  CXXDestructorDataPairList DSOHandleOverride;
  llvm::StringMap<llvm_compat::JITTargetAddress> overrides;
};
} // namespace mull