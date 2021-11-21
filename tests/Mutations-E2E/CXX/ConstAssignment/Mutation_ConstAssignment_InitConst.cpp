#include "FixturePaths.h"

#include "mull/Bitcode.h"
#include "mull/MutationPoint.h"
#include "mull/Mutators/CXX/NumberMutators.h"
#include "mull/Mutators/ScalarValueMutator.h"

#include <clang/Frontend/ASTUnit.h>
#include <clang/Tooling/Tooling.h>
#include <llvm/IR/Module.h>

#include "Helpers/MutationTestBed.h"

#include <gtest/gtest.h>

#include <regex>

using namespace mull;
using namespace mull::cxx;
using namespace mull_test;
using namespace llvm;

static FilePathFilter nullPathFilter;

static const std::string testCode = std::string(R"(///
int foo() {
  int var = 0;
  return var;
}
)");

TEST(Mutation_ConstAssignment_InitConst, End_2_End) {
  NumberInitConst mutator;

  MutationTestBed mutationTestBed;

  std::unique_ptr<MutationArtefact> artefact = mutationTestBed.generate(testCode, mutator);

  std::vector<MutationPoint *> nonJunkMutationPoints = artefact->getNonJunkMutationPoints();
  std::vector<MutationPoint *> junkMutationPoints = artefact->getJunkMutationPoints();

  ASSERT_EQ(junkMutationPoints.size(), 0);
  ASSERT_EQ(nonJunkMutationPoints.size(), 1);

  {
    MutationPoint &mutationPoint = *nonJunkMutationPoints.at(0);

    auto const dumpRegex =
      std::regex("Mutation Point: cxx_init_const /in-memory-file.cc:3:7");
    ASSERT_TRUE(std::regex_search(mutationPoint.dump(), dumpRegex));
  }
}

TEST(Mutation_ConstAssignment_InitConst, DoesNotApply_ConstVars) {
  const std::string constTestCode = std::string(R"(///
int foo() {
  const int var = 0;
  return var;
}
)");

  NumberInitConst mutator;

  MutationTestBed mutationTestBed;

  std::unique_ptr<MutationArtefact> artefact = mutationTestBed.generate(constTestCode, mutator);

  std::vector<MutationPoint *> nonJunkMutationPoints = artefact->getNonJunkMutationPoints();
  std::vector<MutationPoint *> junkMutationPoints = artefact->getJunkMutationPoints();

  ASSERT_EQ(nonJunkMutationPoints.size(), 0);
  ASSERT_EQ(junkMutationPoints.size(), 1);
}

TEST(Mutation_ConstAssignment_InitConst, DoesNotApply_ConstExprVars) {
  const std::string constTestCode = std::string(R"(///
int foo() {
  constexpr int var = 0;
  return var;
}
)");

  NumberInitConst mutator;

  MutationTestBed mutationTestBed;

  std::unique_ptr<MutationArtefact> artefact = mutationTestBed.generate(constTestCode, mutator);

  std::vector<MutationPoint *> nonJunkMutationPoints = artefact->getNonJunkMutationPoints();
  std::vector<MutationPoint *> junkMutationPoints = artefact->getJunkMutationPoints();

  ASSERT_EQ(nonJunkMutationPoints.size(), 0);
  ASSERT_EQ(junkMutationPoints.size(), 1);
}
