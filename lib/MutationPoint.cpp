#include "MutationPoint.h"
#include "Toolchain/Compiler.h"
#include "ModuleLoader.h"

#include "Mutators/Mutator.h"
#include <llvm/Transforms/Utils/Cloning.h>

using namespace llvm;
using namespace mull;
using namespace std;

#pragma mark - MutationPointAddress

Instruction &MutationPointAddress::findInstruction(Module *module) {
  llvm::Function &function = *(std::next(module->begin(), getFnIndex()));
  llvm::BasicBlock &bb = *(std::next(function.begin(), getBBIndex()));
  llvm::Instruction &instruction = *(std::next(bb.begin(), getIIndex()));

  return instruction;
}

int MutationPointAddress::getFunctionIndex(Function *function) {
  auto PM = function->getParent();

  auto FII = std::find_if(PM->begin(), PM->end(),
                          [function] (llvm::Function &f) {
                            return &f == function;
                          });

  assert(FII != PM->end() && "Expected function to be found in module");
  int FIndex = std::distance(PM->begin(), FII);

  return FIndex;
}

void
MutationPointAddress::enumerateInstructions(
  Function &function, const std::function <void (Instruction &,
                                                 int,
                                                 int)>& block) {

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

#pragma mark - MutationPoint

MutationPoint::MutationPoint(Mutator *mutator,
                             MutationPointAddress Address,
                             Value *Val,
                             MullModule *m,
                             std::string diagnostics,
                             const SourceLocation &location) :
  mutator(mutator), Address(Address), OriginalValue(Val),
  module(m), diagnostics(diagnostics), sourceLocation(location), reachableTests()
{
  string moduleID = module->getUniqueIdentifier();
  string addressID = Address.getIdentifier();
  string mutatorID = mutator->getUniqueIdentifier();

  uniqueIdentifier = moduleID + "_" + addressID + "_" + mutatorID;
}

MutationPoint::~MutationPoint() {}

Mutator *MutationPoint::getMutator() {
  return mutator;
}

MutationPointAddress MutationPoint::getAddress() {
  return Address;
}

Value *MutationPoint::getOriginalValue() {
  return OriginalValue;
}

MullModule *MutationPoint::getOriginalModule() {
  return module;
}

Mutator *MutationPoint::getMutator() const {
  return mutator;
}

MutationPointAddress MutationPoint::getAddress() const {
  return Address;
}

Value *MutationPoint::getOriginalValue() const {
  return OriginalValue;
}

MullModule *MutationPoint::getOriginalModule() const {
  return module;
}

void MutationPoint::addReachableTest(Test *test, int distance) {
  reachableTests.push_back(make_pair(test, distance));
}

void MutationPoint::applyMutation(MullModule &module) {
  mutator->applyMutation(module.getModule(), Address);
}

const std::vector<std::pair<Test *, int>> &MutationPoint::getReachableTests() const {
  return reachableTests;
}

std::string MutationPoint::getUniqueIdentifier() {
  return uniqueIdentifier;
}

std::string MutationPoint::getUniqueIdentifier() const {
  return uniqueIdentifier;
}

const std::string &MutationPoint::getDiagnostics() {
  return diagnostics;
}

const std::string &MutationPoint::getDiagnostics() const {
  return diagnostics;
}

const SourceLocation &MutationPoint::getSourceLocation() const {
  return sourceLocation;
}
