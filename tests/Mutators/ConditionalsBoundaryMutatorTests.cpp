#include "FixturePaths.h"
#include "mull/BitcodeLoader.h"
#include "mull/Config/Configuration.h"
#include "mull/Filter.h"
#include "mull/MutationPoint.h"
#include "mull/MutationsFinder.h"
#include "mull/Mutators/ConditionalsBoundaryMutator.h"
#include "mull/Program/Program.h"
#include "mull/Testee.h"
#include "mull/Toolchain/Compiler.h"
#include "mull/Toolchain/Toolchain.h"

#include <llvm/IR/Instructions.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/Transforms/Utils/Cloning.h>

#include "gtest/gtest.h"

using namespace mull;
using namespace llvm;

TEST(ConditionalsBoundaryMutator, findMutations) {
  LLVMContext llvmContext;
  BitcodeLoader loader;
  auto bitcodeFile = loader.loadBitcodeAtPath(
      fixtures::mutators_boundary_module_bc_path(), llvmContext);
  auto module = bitcodeFile->getModule();

  std::vector<std::unique_ptr<Bitcode>> bitcode;
  bitcode.push_back(std::move(bitcodeFile));
  Program program({}, {}, std::move(bitcode));
  Configuration configuration;

  std::vector<std::unique_ptr<Mutator>> mutators;
  mutators.emplace_back(make_unique<ConditionalsBoundaryMutator>());
  MutationsFinder finder(std::move(mutators), configuration);
  Filter filter;

  std::vector<std::unique_ptr<Testee>> testees;
  for (auto &function : *module) {
    testees.emplace_back(make_unique<Testee>(&function, nullptr, 1));
  }
  auto mergedTestees = mergeTestees(testees);
  std::vector<MutationPoint *> points =
      finder.getMutationPoints(program, mergedTestees, filter);

  ASSERT_EQ(points.size(), 7U);
}

TEST(ConditionalsBoundaryMutator, applyMutations) {
  LLVMContext llvmContext;
  BitcodeLoader loader;
  auto bitcodeFile = loader.loadBitcodeAtPath(
      fixtures::mutators_boundary_module_bc_path(), llvmContext);
  auto borrowedBitcode = bitcodeFile.get();
  auto module = borrowedBitcode->getModule();

  std::vector<std::unique_ptr<Bitcode>> bitcode;
  bitcode.push_back(std::move(bitcodeFile));
  Program program({}, {}, std::move(bitcode));
  Configuration configuration;

  std::vector<std::unique_ptr<Mutator>> mutators;
  mutators.emplace_back(make_unique<ConditionalsBoundaryMutator>());
  MutationsFinder finder(std::move(mutators), configuration);
  Filter filter;

  std::vector<std::unique_ptr<Testee>> testees;
  for (auto &function : *module) {
    testees.emplace_back(make_unique<Testee>(&function, nullptr, 1));
  }
  auto mergedTestees = mergeTestees(testees);

  std::vector<MutationPoint *> points =
      finder.getMutationPoints(program, mergedTestees, filter);

  for (auto point : points) {
    ValueToValueMapTy map;
    auto mutatedFunction = CloneFunction(point->getOriginalFunction(), map);

    Instruction *originalInstruction =
        &point->getAddress().findInstruction(point->getOriginalFunction());
    point->setMutatedFunction(mutatedFunction);
    point->applyMutation();
    Instruction *mutatedInstruction =
        &point->getAddress().findInstruction(mutatedFunction);

    if (ConditionalsBoundaryMutator::isGT(originalInstruction)) {
      ASSERT_TRUE(ConditionalsBoundaryMutator::isGTE(mutatedInstruction));
    } else if (ConditionalsBoundaryMutator::isGTE(originalInstruction)) {
      ASSERT_TRUE(ConditionalsBoundaryMutator::isGT(mutatedInstruction));
    } else if (ConditionalsBoundaryMutator::isLTE(originalInstruction)) {
      ASSERT_TRUE(ConditionalsBoundaryMutator::isLT(mutatedInstruction));
    } else if (ConditionalsBoundaryMutator::isLT(originalInstruction)) {
      ASSERT_TRUE(ConditionalsBoundaryMutator::isLTE(mutatedInstruction));
    }
  }

  ASSERT_EQ(points.size(), 7U);
}
