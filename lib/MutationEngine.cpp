#include "MutationEngine.h"

#include "MutationPoint.h"
#include "TestFinders/SimpleTestFinder.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Module.h"

using namespace llvm;
using namespace Mutang;

void MutationEngine::applyMutation(MutationPoint &MP) {
  MP.applyMutation();
}

void MutationEngine::revertMutation(MutationPoint &MP) {
  MP.revertMutation();
}
