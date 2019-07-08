#include "mull/Mutators/ScalarValueMutator.h"
#include "FixturePaths.h"
#include "TestModuleFactory.h"
#include "mull/Config/Configuration.h"
#include "mull/Filter.h"
#include "mull/ModuleLoader.h"
#include "mull/MutationPoint.h"
#include "mull/MutationsFinder.h"
#include "mull/Program/Program.h"
#include "mull/Testee.h"
#include "mull/Toolchain/Compiler.h"
#include "mull/Toolchain/Toolchain.h"

#include <llvm/IR/Instructions.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>

#include "gtest/gtest.h"

using namespace mull;
using namespace llvm;

TEST(ScalarValueMutator, getMutationPoint) {
  LLVMContext llvmContext;
  ModuleLoader loader;
  auto mullModule = loader.loadModuleAtPath(
      fixtures::mutators_scalar_value_module_bc_path(), llvmContext);

  std::vector<std::unique_ptr<MullModule>> modules;
  modules.push_back(std::move(mullModule));
  Program program({}, {}, std::move(modules));

  auto scalarValueFunction = program.lookupDefinedFunction("scalar_value");
  std::vector<std::unique_ptr<Testee>> testees;
  testees.emplace_back(make_unique<Testee>(scalarValueFunction, nullptr, 1));
  auto mergedTestees = mergeTestees(testees);
  Configuration configuration;

  std::vector<std::unique_ptr<Mutator>> mutators;
  mutators.emplace_back(make_unique<ScalarValueMutator>());
  MutationsFinder finder(std::move(mutators), configuration);
  Filter filter;

  std::vector<MutationPoint *> mutationPoints =
      finder.getMutationPoints(program, mergedTestees, filter);

  ASSERT_EQ(mutationPoints.size(), 4U);

  ASSERT_TRUE(isa<StoreInst>(mutationPoints[0]->getOriginalValue()));
  ASSERT_TRUE(isa<StoreInst>(mutationPoints[1]->getOriginalValue()));
  ASSERT_TRUE(isa<BinaryOperator>(mutationPoints[2]->getOriginalValue()));
  ASSERT_TRUE(isa<BinaryOperator>(mutationPoints[3]->getOriginalValue()));

  ASSERT_EQ(mutationPoints[0]->getAddress().getFnIndex(), 0);
  ASSERT_EQ(mutationPoints[0]->getAddress().getBBIndex(), 0);
  ASSERT_EQ(mutationPoints[0]->getAddress().getIIndex(), 4);

  ASSERT_EQ(mutationPoints[1]->getAddress().getFnIndex(), 0);
  ASSERT_EQ(mutationPoints[1]->getAddress().getBBIndex(), 0);
  ASSERT_EQ(mutationPoints[1]->getAddress().getIIndex(), 5);

  ASSERT_EQ(mutationPoints[2]->getAddress().getFnIndex(), 0);
  ASSERT_EQ(mutationPoints[2]->getAddress().getBBIndex(), 0);
  ASSERT_EQ(mutationPoints[2]->getAddress().getIIndex(), 9);

  ASSERT_EQ(mutationPoints[3]->getAddress().getFnIndex(), 0);
  ASSERT_EQ(mutationPoints[3]->getAddress().getBBIndex(), 0);
  ASSERT_EQ(mutationPoints[3]->getAddress().getIIndex(), 12);
}

TEST(DISABLED_ScalarValueMutator, failingMutationPoint) {
  LLVMContext llvmContext;
  ModuleLoader loader;
  auto mullModule = loader.loadModuleAtPath(
      fixtures::hardcode_openssl_bio_enc_test_oll_path(), llvmContext);

  MutationPointAddress address(15, 10, 7);
  ScalarValueMutator mutator;
  MutationPoint point(&mutator, address, nullptr, "diagnostics",
                      SourceLocation::nullSourceLocation(), mullModule.get());

  Configuration configuration;
  Toolchain toolchain(configuration);
  //  auto mutant = point.cloneModuleAndApplyMutation();
  //  toolchain.compiler().compileModule(mutant.get());
}
