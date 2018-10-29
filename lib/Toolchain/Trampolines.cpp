#include "Toolchain/Trampolines.h"
#include "Toolchain/JITEngine.h"
#include "Toolchain/Mangler.h"

using namespace mull;

Trampolines::Trampolines(const std::vector<std::string> &trampolineNames) : trampolineNames(trampolineNames) {}

Trampolines::~Trampolines() {
  for (auto &pair : _trampolines) {
    delete pair.second;
  }
}

void Trampolines::fixupOriginalFunctions(JITEngine &jit) {
  for (auto &pair : _trampolines) {
    auto &name = pair.first;
    auto originalName = name.substr(0, name.length() - std::string("_trampoline").length()) + "_original";
    auto it = _trampolines.find(name);
    assert(it != _trampolines.end());
    uint64_t *trampoline = it->second;
    auto address = llvm_compat::JITSymbolAddress(jit.getSymbol(originalName));
    assert(address);
    *trampoline = address;
  }
}

uint64_t *Trampolines::findTrampoline(const std::string &name) {
  auto it = _trampolines.find(name);
  if (it == _trampolines.end()) {
    return nullptr;
  }
  return it->second;
}

void Trampolines::allocateTrampolines(Mangler &mangler) {
  for (auto &name : trampolineNames) {
    _trampolines.insert(std::make_pair(mangler.getNameWithPrefix(name), new uint64_t));
  }
}
