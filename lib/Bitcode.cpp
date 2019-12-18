#include "mull/Bitcode.h"

#include "LLVMCompatibility.h"
#include "mull/Diagnostics/Diagnostics.h"
#include "mull/MutationPoint.h"
#include "mull/Parallelization/Parallelization.h"

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/MD5.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/Path.h>
#include <llvm/Support/SourceMgr.h>
#include <llvm/Transforms/Utils/Cloning.h>
#include <assert.h>

using namespace mull;
using namespace llvm;
using namespace std;

Bitcode::Bitcode(std::unique_ptr<llvm::Module> module)
    : module(std::move(module)), uniqueIdentifier("") {}

Bitcode::Bitcode(std::unique_ptr<llvm::Module> module, std::unique_ptr<llvm::MemoryBuffer> buffer,
                 const std::string &md5)
    : module(std::move(module)), buffer(std::move(buffer)) {
  uniqueIdentifier = llvm::sys::path::stem(this->module->getModuleIdentifier()).str() + "_" + md5;
}

std::unique_ptr<Bitcode> Bitcode::clone(LLVMContext &context, Diagnostics &diagnostics) {
  assert(buffer.get() && "Cannot clone non-original module");
  auto clone = llvm_compat::parseBitcode(buffer->getMemBufferRef(), context);
  if (!clone) {
    diagnostics.error("Cannot clone module \n");
    return nullptr;
  }

  clone->setModuleIdentifier(module->getModuleIdentifier());

  return make_unique<Bitcode>(std::move(clone), std::unique_ptr<MemoryBuffer>(), "");
}

llvm::Module *Bitcode::getModule() {
  assert(module);
  return module.get();
}

llvm::Module *Bitcode::getModule() const {
  assert(module);
  return module.get();
}

std::string Bitcode::getUniqueIdentifier() {
  return uniqueIdentifier;
}

std::string Bitcode::getUniqueIdentifier() const {
  return uniqueIdentifier;
}

void Bitcode::addMutation(MutationPoint *point) {
  auto function = point->getOriginalFunction();
  mutationPoints[function].push_back(point);
}

std::string Bitcode::getInstrumentedUniqueIdentifier() const {
  return getUniqueIdentifier() + "_instrumented";
}

std::string Bitcode::getMutatedUniqueIdentifier() const {
  if (mutationPoints.empty()) {
    return getUniqueIdentifier();
  }

  std::vector<std::string> mutationPointsIds;
  for (const auto &mutationPair : mutationPoints) {
    for (auto point : mutationPair.second) {
      mutationPointsIds.push_back(point->getUniqueIdentifier());
    }
  }

  std::sort(mutationPointsIds.begin(), mutationPointsIds.end());

  MD5 hasher;
  for (auto &id : mutationPointsIds) {
    hasher.update(id);
  }

  MD5::MD5Result hash{};
  hasher.final(hash);
  SmallString<32> result;
  MD5::stringifyResult(hash, result);

  return (getUniqueIdentifier() + "_" + result + "_mutated").str();
}

std::map<llvm::Function *, std::vector<MutationPoint *>> &Bitcode::getMutationPointsMap() {
  return mutationPoints;
}
