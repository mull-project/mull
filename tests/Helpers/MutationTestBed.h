#pragma once

#include "InMemoryFixture.h"
#include "MutationArtefact.h"

#include "mull/Diagnostics/Diagnostics.h"
#include "mull/Filters/FilePathFilter.h"

#include <llvm/IR/LLVMContext.h>

#include <memory>

namespace mull {
class Mutator;
}

namespace mull_test {

class MutationTestBed {
public:
  MutationTestBed();
  std::unique_ptr<MutationArtefact> generate(const std::string &sourceCode, mull::Mutator &mutator);

private:
  InMemoryFixture fixtureGenerator;
  mull::FilePathFilter nullPathFilter;
  mull::Diagnostics diagnostics;
};

} // namespace mull_test
