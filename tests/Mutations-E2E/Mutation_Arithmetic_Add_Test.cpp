#include "FixturePaths.h"

#include "mull/AST/ASTVisitor.h"
#include "mull/MutationPoint.h"
#include "mull/Mutators/CXX/ArithmeticMutators.h"

#include "Helpers/MutationTestBed.h"

#include <gtest/gtest.h>

using namespace mull;
using namespace mull_test;
using namespace llvm;

static const std::string testCode = std::string(R"(///
int add(int a, int b) {
  return a + b;
}
)");

TEST(Mutation_Arithmetic_AddToSub, End_2_End) {
  mull::cxx::AddToSub mutator;

  MutationTestBed mutationTestBed;

  std::unique_ptr<MutationArtefact> artefact = mutationTestBed.generate(testCode, mutator);

  /// 1. AST Assertions
  LineColumnHash locationHash = lineColumnHash(3, 12);

  SingleASTUnitMutations singleUnitMutations = artefact->getASTMutations();
  ASSERT_EQ(singleUnitMutations.size(), 1U);
  ASSERT_EQ(singleUnitMutations.count(MutatorKind::CXX_AddToSub), 1U);

  ASSERT_EQ(singleUnitMutations[MutatorKind::CXX_AddToSub].size(), 1U);
  ASSERT_EQ(singleUnitMutations[MutatorKind::CXX_AddToSub].count(locationHash), 1U);

  /// 2. IR and Junk Detection Assertions
  std::vector<MutationPoint *> nonJunkMutationPoints = artefact->getNonJunkMutationPoints();
  std::vector<MutationPoint *> junkMutationPoints = artefact->getJunkMutationPoints();

  ASSERT_EQ(nonJunkMutationPoints.size(), 1);
  ASSERT_EQ(junkMutationPoints.size(), 0);

  MutationPoint &mutationPoint = *nonJunkMutationPoints.at(0);

  auto const dumpRegex = std::regex("Mutation Point: cxx_add_to_sub /in-memory-file.cc:3:12");
  ASSERT_TRUE(std::regex_search(mutationPoint.dump(), dumpRegex));
}
