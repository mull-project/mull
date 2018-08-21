#include "MullModule.h"
#include "Logger.h"
#include "LLVMCompatibility.h"
#include "MutationPoint.h"

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/SourceMgr.h>
#include <llvm/Support/Path.h>
#include <llvm/Transforms/Utils/Cloning.h>
#include <llvm/Support/MD5.h>

using namespace mull;
using namespace llvm;
using namespace std;

MullModule::MullModule(std::unique_ptr<llvm::Module> llvmModule)
  : module(std::move(llvmModule)),
    uniqueIdentifier("")
{
}

MullModule::MullModule(std::unique_ptr<llvm::Module> llvmModule,
                       const std::string &md5,
                       const std::string &path)
: module(std::move(llvmModule)), modulePath(path)
{
  uniqueIdentifier =
    llvm::sys::path::stem(module->getModuleIdentifier()).str() + "_" + md5;
}

std::unique_ptr<MullModule> MullModule::clone(LLVMContext &context) {
  auto bufferOrError = MemoryBuffer::getFile(modulePath);
  if (!bufferOrError) {
    Logger::error() << "MullModule::clone> Can't load module " << modulePath << '\n';
    return nullptr;
  }

  auto llvmModule = parseBitcodeFile(bufferOrError->get()->getMemBufferRef(), context);
  if (!llvmModule) {
    Logger::error() << "MullModule::clone> Can't load module " << modulePath << '\n';
    return nullptr;
  }

  auto module = make_unique<MullModule>(std::move(llvmModule.get()), "", modulePath);
  return module;
}

llvm::Module *MullModule::getModule() {
  assert(module);
  return module.get();
}

llvm::Module *MullModule::getModule() const {
  assert(module);
  return module.get();
}

std::string MullModule::getUniqueIdentifier() {
  return uniqueIdentifier;
}

std::string MullModule::getUniqueIdentifier() const {
  return uniqueIdentifier;
}

std::vector<std::string> MullModule::prepareMutations() {
  std::vector<std::string> mutatedFunctionNames;

  for (auto pair : mutationPoints) {
    auto original = pair.first;
    auto anyPoint = pair.second.front();
    mutatedFunctionNames.push_back(anyPoint->getTrampolineName());
    for (auto point : pair.second) {
      ValueToValueMapTy map;
      auto mutatedFunction = CloneFunction(original, map);
      point->setMutatedFunction(mutatedFunction);
    }
    ValueToValueMapTy map;
    auto originalCopy = CloneFunction(original, map);
    originalCopy->setName(anyPoint->getOriginalFunctionName());
    original->deleteBody();

    std::vector<Value *> args;

    for (auto &arg : original->args()) {
      args.push_back(&arg);
    }

    auto trampoline = module->getOrInsertGlobal(anyPoint->getTrampolineName(), original->getFunctionType()->getPointerTo());
    BasicBlock *block = BasicBlock::Create(module->getContext(), "indirect_call", original);
    auto loadValue = new LoadInst(trampoline, "indirect_function_pointer", block);
    auto callInst = CallInst::Create(loadValue, args, "indirect_function_call", block);
    ReturnInst::Create(module->getContext(), callInst, block);
  }

  return mutatedFunctionNames;
}

void MullModule::addMutation(MutationPoint *point) {
  std::lock_guard<std::mutex> guard(mutex);
  auto function = point->getOriginalFunction();
  mutationPoints[function].push_back(point);
}

std::string MullModule::getInstrumentedUniqueIdentifier() const {
  return getUniqueIdentifier() + "_instrumented";
}

std::string MullModule::getMutatedUniqueIdentifier() const {
  if (mutationPoints.empty()) {
    return getUniqueIdentifier();
  }

  std::vector<std::string> mutationPointsIds;
  for (auto mutationPair: mutationPoints) {
    for (auto point : mutationPair.second) {
      mutationPointsIds.push_back(point->getUniqueIdentifier());
    }
  }

  std::sort(mutationPointsIds.begin(), mutationPointsIds.end());

  MD5 hasher;
  for (auto &id : mutationPointsIds) {
    hasher.update(id);
  }

  MD5::MD5Result hash;
  hasher.final(hash);
  SmallString<32> result;
  MD5::stringifyResult(hash, result);

  return (getUniqueIdentifier() + "_" + result + "_mutated").str();
}

