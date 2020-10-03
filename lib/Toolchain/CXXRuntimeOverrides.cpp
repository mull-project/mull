#include "mull/Toolchain/CXXRuntimeOverrides.h"

#include <cstdint>

int mull::CXXRuntimeOverrides::CXAAtExitOverride(DestructorPtr destructor, void *arg,
                                                 void *DSOHandle) {
  auto &CXXDestructorDataPairs =
      *reinterpret_cast<mull::CXXRuntimeOverrides::CXXDestructorDataPairList *>(DSOHandle);
  CXXDestructorDataPairs.emplace_back(destructor, arg);
  return 0;
}

template <typename T> uint64_t toTargetAddress(T *pointer) {
  return static_cast<uint64_t>(reinterpret_cast<uintptr_t>(pointer));
}

void mull::CXXRuntimeOverrides::runDestructors() {
  auto &CXXDestructorDataPairs = DSOHandleOverride;
  for (auto &P : CXXDestructorDataPairs) {
    P.first(P.second);
  }
  CXXDestructorDataPairs.clear();
}

void mull::CXXRuntimeOverrides::addOverride(const std::string &name,
                                            llvm::JITTargetAddress address) {
  overrides.insert(std::make_pair(name, address));
}

mull::CXXRuntimeOverrides::CXXRuntimeOverrides(mull::Mangler &mangler) {
  addOverride(mangler.getNameWithPrefix("__dso_handle"), toTargetAddress(&DSOHandleOverride));
  addOverride(mangler.getNameWithPrefix("__cxa_atexit"), toTargetAddress(&CXAAtExitOverride));
}

llvm::JITSymbol mull::CXXRuntimeOverrides::searchOverrides(const std::string &name) {
  auto I = overrides.find(name);
  if (I != overrides.end())
    return llvm::JITSymbol(I->second, llvm::JITSymbolFlags::Exported);
  return nullptr;
}
