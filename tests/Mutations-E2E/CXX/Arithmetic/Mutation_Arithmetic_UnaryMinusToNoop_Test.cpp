#include "FixturePaths.h"

#include "mull/AST/ASTVisitor.h"
#include "mull/Bitcode.h"
#include "mull/MutationPoint.h"
#include "mull/Mutators/CXX/ArithmeticMutators.h"

#include "Helpers/MutationTestBed.h"

#include <gtest/gtest.h>

using namespace mull;
using namespace mull_test;
using namespace llvm;

static const std::string testCode = std::string(R"(///
int foo(int x) {
  return -x;
}
)");

TEST(Mutation_Arithmetic_UnaryMinusToNoop, End_2_End) {
  mull::cxx::UnaryMinusToNoop mutator;

  MutationTestBed mutationTestBed;

  std::unique_ptr<MutationArtefact> artefact = mutationTestBed.generate(testCode, mutator);

  /// 1. AST Assertions
  LineColumnHash locationHash = lineColumnHash(3, 10);

  SingleASTUnitMutations singleUnitMutations = artefact->getASTMutations();
  ASSERT_EQ(singleUnitMutations.size(), 1U);
  ASSERT_EQ(singleUnitMutations.count(MutatorKind::CXX_UnaryMinusToNoop), 1U);

  ASSERT_EQ(singleUnitMutations[MutatorKind::CXX_UnaryMinusToNoop].size(), 1U);
  ASSERT_EQ(singleUnitMutations[MutatorKind::CXX_UnaryMinusToNoop].count(locationHash), 1U);

  /// 2. IR and Junk Detection Assertions
  std::vector<MutationPoint *> nonJunkMutationPoints = artefact->getNonJunkMutationPoints();
  std::vector<MutationPoint *> junkMutationPoints = artefact->getJunkMutationPoints();

  ASSERT_EQ(nonJunkMutationPoints.size(), 1);
  ASSERT_EQ(junkMutationPoints.size(), 0);

  MutationPoint &mutationPoint = *nonJunkMutationPoints.at(0);

  auto const dumpRegex = std::regex("Mutation Point: cxx_minus_to_noop /in-memory-file.cc:3:10");
  ASSERT_TRUE(std::regex_search(mutationPoint.dump(), dumpRegex));
}
