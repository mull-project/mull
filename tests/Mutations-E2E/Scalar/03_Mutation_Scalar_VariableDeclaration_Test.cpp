#include "FixturePaths.h"

#include "mull/Bitcode.h"
#include "mull/MutationPoint.h"
#include <mull/Mutators/ScalarValueMutator.h>

#include <clang/Frontend/ASTUnit.h>
#include <clang/Tooling/Tooling.h>
#include <llvm/IR/Module.h>

#include "Helpers/MutationTestBed.h"

#include <gtest/gtest.h>

using namespace mull;
using namespace mull_test;
using namespace llvm;

static FilePathFilter nullPathFilter;

static const std::string testCode = std::string(R"(///
int foo() {
  int var = 0;
  return var;
}
)");

TEST(Mutation_Scalar_VariableDeclaration, End_2_End) {
  mull::ScalarValueMutator mutator;

  MutationTestBed mutationTestBed;

  std::unique_ptr<MutationArtefact> artefact = mutationTestBed.generate(testCode, mutator);

  /// 1. AST Assertions
  LineColumnHash locationHash = lineColumnHash(3, 7);

  SingleASTUnitMutations singleUnitMutations = artefact->getASTMutations();
  ASSERT_EQ(singleUnitMutations.size(), 1U);
  ASSERT_EQ(singleUnitMutations.count(MutatorKind::ScalarValueMutator), 1U);

  ASSERT_EQ(singleUnitMutations[MutatorKind::ScalarValueMutator].size(), 1U);
  ASSERT_EQ(singleUnitMutations[MutatorKind::ScalarValueMutator].count(locationHash), 1U);

  /// 2. IR and Junk Detection Assertions
  std::vector<MutationPoint *> nonJunkMutationPoints = artefact->getNonJunkMutationPoints();
  std::vector<MutationPoint *> junkMutationPoints = artefact->getJunkMutationPoints();

  ASSERT_EQ(nonJunkMutationPoints.size(), 1);
  ASSERT_EQ(junkMutationPoints.size(), 0);

  {
    MutationPoint &mutationPoint = *nonJunkMutationPoints.at(0);

    auto const dumpRegex =
      std::regex("Mutation Point: scalar_value_mutator /in-memory-file.cc:3:7");
    ASSERT_TRUE(std::regex_search(mutationPoint.dump(), dumpRegex));
  }
}

TEST(Mutation_Scalar_VariableDeclaration, DoesNotApply_ConstVars) {
  const std::string constTestCode = std::string(R"(///
int foo() {
  const int var = 0;
  return var;
}
)");

  mull::ScalarValueMutator mutator;

  MutationTestBed mutationTestBed;

  std::unique_ptr<MutationArtefact> artefact = mutationTestBed.generate(constTestCode, mutator);

  /// 1. AST Assertions
  SingleASTUnitMutations singleUnitMutations = artefact->getASTMutations();
  ASSERT_EQ(singleUnitMutations.size(), 0U);
  ASSERT_EQ(singleUnitMutations.count(MutatorKind::ScalarValueMutator), 0U);

  /// 2. IR and Junk Detection Assertions
  std::vector<MutationPoint *> nonJunkMutationPoints = artefact->getNonJunkMutationPoints();
  std::vector<MutationPoint *> junkMutationPoints = artefact->getJunkMutationPoints();

  ASSERT_EQ(nonJunkMutationPoints.size(), 0);
  ASSERT_EQ(junkMutationPoints.size(), 2);
}

TEST(Mutation_Scalar_VariableDeclaration, DoesNotApply_ConstExprVars) {
  const std::string constTestCode = std::string(R"(///
int foo() {
  constexpr int var = 0;
  return var;
}
)");

  mull::ScalarValueMutator mutator;

  MutationTestBed mutationTestBed;

  std::unique_ptr<MutationArtefact> artefact = mutationTestBed.generate(constTestCode, mutator);

  /// 1. AST Assertions
  SingleASTUnitMutations singleUnitMutations = artefact->getASTMutations();
  ASSERT_EQ(singleUnitMutations.size(), 0U);
  ASSERT_EQ(singleUnitMutations.count(MutatorKind::ScalarValueMutator), 0U);

  /// 2. IR and Junk Detection Assertions
  std::vector<MutationPoint *> nonJunkMutationPoints = artefact->getNonJunkMutationPoints();
  std::vector<MutationPoint *> junkMutationPoints = artefact->getJunkMutationPoints();

  ASSERT_EQ(nonJunkMutationPoints.size(), 0);
  ASSERT_EQ(junkMutationPoints.size(), 2);
}