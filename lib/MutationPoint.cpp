#include "mull/MutationPoint.h"

#include "mull/BitcodeLoader.h"
#include "mull/Mutators/Mutator.h"
#include "mull/Reporters/SourceManager.h"
#include "mull/Toolchain/Compiler.h"

#include <llvm/IR/Function.h>
#include <llvm/Transforms/Utils/Cloning.h>

#include <assert.h>
#include <iomanip>
#include <sstream>
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

llvm::Instruction &MutationPointAddress::findInstruction(llvm::Function *function) const {
  llvm::BasicBlock &bb = *(std::next(function->begin(), getBBIndex()));
  llvm::Instruction &instruction = *(std::next(bb.begin(), getIIndex()));

  return instruction;
}

MutationPointAddress::MutationPointAddress(int FnIndex, int BBIndex, int IIndex)
    : functionIndex(FnIndex), basicBlockIndex(BBIndex), instructionIndex(IIndex),
      identifier(std::to_string(functionIndex) + "_" + std::to_string(basicBlockIndex) + "_" +
                 std::to_string(instructionIndex)) {}

int MutationPointAddress::getFnIndex() const {
  return functionIndex;
}

int MutationPointAddress::getBBIndex() const {
  return basicBlockIndex;
}

int MutationPointAddress::getIIndex() const {
  return instructionIndex;
}

std::string MutationPointAddress::getIdentifier() const {
  return identifier;
}

template <typename Container, typename Value>
static size_t getIndex(Container &container, Value *value) {
  size_t index = 0;
  auto begin = container.begin();
  auto end = container.end();
  for (; begin != end; begin++, index++) {
    if (&*begin == value) {
      break;
    }
  }
  return index;
}

const MutationPointAddress
MutationPointAddress::addressFromInstruction(const llvm::Instruction *instruction) {
  return MutationPointAddress(
      getIndex(instruction->getModule()->getFunctionList(), instruction->getFunction()),
      getIndex(instruction->getFunction()->getBasicBlockList(), instruction->getParent()),
      getIndex(instruction->getParent()->getInstList(), instruction));
}

#pragma mark - MutationPoint

MutationPoint::MutationPoint(Mutator *mutator, irm::IRMutation *irMutator,
                             llvm::Instruction *instruction, std::string replacement, Bitcode *m,
                             std::string diagnostics)
    : mutator(mutator), irMutator(irMutator),
      address(MutationPointAddress::addressFromInstruction(instruction)), bitcode(m),
      originalFunction(instruction->getFunction()), mutatedFunction(nullptr),
      diagnostics(std::move(diagnostics)), replacement(replacement),
      sourceLocation(SourceLocation::locationFromInstruction(instruction)), reachableTests(),
      uniqueIdentifier(bitcode->getUniqueIdentifier() + "_" + address.getIdentifier() + "_" +
                       mutator->getUniqueIdentifier()) {}

Mutator *MutationPoint::getMutator() {
  return mutator;
}

Mutator *MutationPoint::getMutator() const {
  return mutator;
}

const MutationPointAddress &MutationPoint::getAddress() const {
  return address;
}

Value *MutationPoint::getOriginalValue() const {
  auto *function = originalFunction;
  if (mutatedFunction != nullptr) {
    function = function->getParent()->getFunction(getOriginalFunctionName());
  }
  assert(function && "The original function should be present");
  return &(address.findInstruction(function));
}

Bitcode *MutationPoint::getBitcode() const {
  return bitcode;
}

void MutationPoint::addReachableTest(Test *test, int distance) {
  reachableTests.emplace_back(test, distance);
}

void MutationPoint::applyMutation() {
  assert(mutatedFunction != nullptr);
  mutator->applyMutation(mutatedFunction, address, irMutator);
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

std::string MutationPoint::getMutatorIdentifier() const {
  return mutator->getUniqueIdentifier();
}

const std::string &MutationPoint::getDiagnostics() {
  return diagnostics;
}

const std::string &MutationPoint::getDiagnostics() const {
  return diagnostics;
}

const std::string &MutationPoint::getReplacement() {
  return replacement;
}

const SourceLocation &MutationPoint::getSourceLocation() const {
  return sourceLocation;
}

Function *MutationPoint::getOriginalFunction() {
  return originalFunction;
}

void MutationPoint::setMutatedFunction(llvm::Function *function) {
  function->setName(getMutatedFunctionName());
  this->mutatedFunction = function;
}

std::string MutationPoint::getTrampolineName() {
  return originalFunction->getName().str() + "_" + bitcode->getUniqueIdentifier() + "_trampoline";
}

std::string MutationPoint::getMutatedFunctionName() {
  return getUniqueIdentifier();
}

std::string MutationPoint::getOriginalFunctionName() const {
  return originalFunction->getName().str() + "_" + bitcode->getUniqueIdentifier() + "_original";
}

std::string MutationPoint::dump() const {
  std::stringstream ss;
  ss << "Mutation Point: " << getMutator()->getUniqueIdentifier() << " "
     << getSourceLocation().filePath << ":" << getSourceLocation().line << ":"
     << getSourceLocation().column;
  return ss.str();
}

std::string MutationPoint::dumpSourceCodeContext() const {
  std::stringstream ss;

  if (sourceLocation.isNull() || sourceLocation.line == 0 || sourceLocation.column == 0 ||
      sourceLocation.filePath.empty()) {
    ss << "Source code information is unavailable. Possibly a junk mutation.";
    return ss.str();
  }

  SourceManager sourceManager;

  const std::string delimiter = ":";

  const size_t totalLines = sourceManager.getNumberOfLines(sourceLocation);

  const size_t maxLineNumber = std::min(totalLines, (size_t)(sourceLocation.line + 1));

  const size_t maxDigits = (maxLineNumber == 0) ? 1 : (log10(maxLineNumber) + 1);

  auto line = sourceManager.getLine(sourceLocation);
  assert(sourceLocation.column < line.size());

  std::string caret(sourceLocation.column, ' ');
  for (size_t index = 0; index < sourceLocation.column; index++) {
    if (line[index] == '\t') {
      caret[index] = '\t';
    }
  }
  caret[sourceLocation.column - 1] = '^';
  caret.insert(0, std::string(maxDigits + delimiter.size(), ' '));

  if (sourceLocation.line > 1) {
    SourceLocation previousLineLocation(sourceLocation.unitDirectory,
                                        sourceLocation.unitFilePath,
                                        sourceLocation.directory,
                                        sourceLocation.filePath,
                                        sourceLocation.line - 1,
                                        sourceLocation.column);
    auto previousLine = sourceManager.getLine(previousLineLocation);
    ss << std::setw(maxDigits) << previousLineLocation.line << delimiter << previousLine;
  }

  ss << std::setw(maxDigits) << sourceLocation.line << delimiter << line << caret << "\n";

  if (sourceLocation.line < totalLines) {
    SourceLocation nextLineLocation(sourceLocation.unitDirectory,
                                    sourceLocation.unitFilePath,
                                    sourceLocation.directory,
                                    sourceLocation.filePath,
                                    sourceLocation.line + 1,
                                    sourceLocation.column);
    auto nextLine = sourceManager.getLine(nextLineLocation);
    ss << std::setw(maxDigits) << nextLineLocation.line << delimiter << nextLine;
  }

  return ss.str();
}
