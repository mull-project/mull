#include "mull/MutationPoint.h"

#include "mull/ModuleLoader.h"
#include "mull/Mutators/Mutator.h"
#include "mull/Toolchain/Compiler.h"

#include <llvm/IR/Function.h>
#include <llvm/Transforms/Utils/Cloning.h>

#include <utility>

using namespace llvm;
using namespace mull;
using namespace std;

#pragma mark - MutationPointAddress

Instruction &MutationPointAddress::findInstruction(Module *module) const {
  llvm::Function &function = *(std::next(module->begin(), getFnIndex()));
  llvm::BasicBlock &bb = *(std::next(function.begin(), getBBIndex()));
  llvm::Instruction &instruction = *(std::next(bb.begin(), getIIndex()));

  return instruction;
}

Function &MutationPointAddress::findFunction(Module *module) const {
  llvm::Function &function = *(std::next(module->begin(), getFnIndex()));
  return function;
}

int MutationPointAddress::getFunctionIndex(Function *function) {
  auto PM = function->getParent();

  auto FII =
      std::find_if(PM->begin(), PM->end(),
                   [function](llvm::Function &f) { return &f == function; });

  assert(FII != PM->end() && "Expected function to be found in module");
  int FIndex = std::distance(PM->begin(), FII);

  return FIndex;
}

void MutationPointAddress::enumerateInstructions(
    Function &function,
    const std::function<void(Instruction &, int, int)> &block) {

  int basicBlockIndex = 0;
  for (auto &basicBlock : function.getBasicBlockList()) {
    int instructionIndex = 0;

    for (auto &instruction : basicBlock.getInstList()) {
      block(instruction, basicBlockIndex, instructionIndex);

      instructionIndex++;
    }
    basicBlockIndex++;
  }
}

llvm::Instruction &
MutationPointAddress::findInstruction(llvm::Function *function) const {
  llvm::BasicBlock &bb = *(std::next(function->begin(), getBBIndex()));
  llvm::Instruction &instruction = *(std::next(bb.begin(), getIIndex()));

  return instruction;
}

MutationPointAddress::MutationPointAddress(int FnIndex, int BBIndex, int IIndex)
    : FnIndex(FnIndex), BBIndex(BBIndex), IIndex(IIndex) {

  identifier = std::to_string(FnIndex) + "_" + std::to_string(BBIndex) + "_" +
               std::to_string(IIndex);
}

int MutationPointAddress::getFnIndex() const { return FnIndex; }

int MutationPointAddress::getBBIndex() const { return BBIndex; }

int MutationPointAddress::getIIndex() const { return IIndex; }

std::string MutationPointAddress::getIdentifier() { return identifier; }

std::string MutationPointAddress::getIdentifier() const { return identifier; }

#pragma mark - MutationPoint

MutationPoint::MutationPoint(Mutator *mutator, MutationPointAddress address,
                             llvm::Function *function, std::string diagnostics,
                             SourceLocation location, MullModule *m)
    : mutator(mutator), address(address), module(m), originalFunction(function),
      mutatedFunction(nullptr), diagnostics(std::move(diagnostics)),
      sourceLocation(std::move(location)), reachableTests() {
  string moduleID = module->getUniqueIdentifier();
  string addressID = address.getIdentifier();
  string mutatorID = mutator->getUniqueIdentifier();

  uniqueIdentifier = moduleID + "_" + addressID + "_" + mutatorID;
}

Mutator *MutationPoint::getMutator() { return mutator; }

MutationPointAddress MutationPoint::getAddress() { return address; }

Value *MutationPoint::getOriginalValue() const {
  auto *function = originalFunction;
  if (mutatedFunction != nullptr) {
    function = function->getParent()->getFunction(getOriginalFunctionName());
  }
  assert(function && "The original function should be present");
  return &(address.findInstruction(function));
}

MullModule *MutationPoint::getOriginalModule() const { return module; }

void MutationPoint::addReachableTest(Test *test, int distance) {
  reachableTests.emplace_back(test, distance);
}

void MutationPoint::applyMutation() {
  assert(mutatedFunction != nullptr);
  mutator->applyMutation(mutatedFunction, address);
}

const std::vector<std::pair<Test *, int>> &
MutationPoint::getReachableTests() const {
  return reachableTests;
}

std::string MutationPoint::getUniqueIdentifier() { return uniqueIdentifier; }

std::string MutationPoint::getUniqueIdentifier() const {
  return uniqueIdentifier;
}

const std::string &MutationPoint::getDiagnostics() { return diagnostics; }

const std::string &MutationPoint::getDiagnostics() const { return diagnostics; }

const SourceLocation &MutationPoint::getSourceLocation() const {
  return sourceLocation;
}

Function *MutationPoint::getOriginalFunction() { return originalFunction; }

void MutationPoint::setMutatedFunction(llvm::Function *function) {
  function->setName(getMutatedFunctionName());
  this->mutatedFunction = function;
}

std::string MutationPoint::getTrampolineName() {
  return originalFunction->getName().str() + "_" +
         module->getUniqueIdentifier() + "_trampoline";
}

std::string MutationPoint::getMutatedFunctionName() {
  return getUniqueIdentifier();
}

std::string MutationPoint::getOriginalFunctionName() const {
  return originalFunction->getName().str() + "_" +
         module->getUniqueIdentifier() + "_original";
}
