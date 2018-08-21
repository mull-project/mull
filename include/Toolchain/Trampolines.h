#pragma once

#include <string>
#include <vector>
#include <map>

namespace mull {
class JITEngine;
class Mangler;

class Trampolines {
public:
  explicit Trampolines(const std::vector<std::string> &trampolineNames);
  ~Trampolines();

  void allocateTrampolines(Mangler &mangler);

  uint64_t *findTrampoline(const std::string &name);

  void fixupOriginalFunctions(JITEngine &jit);
private:
  const std::vector<std::string> &trampolineNames;
  std::map<std::string, uint64_t *> _trampolines;
};
}
