#include "FixturePaths.h"

#include "mull/AST/ASTVisitor.h"
#include "mull/Bitcode.h"
#include "mull/MutationPoint.h"
#include "mull/Mutators/CXX/RemoveNegation.h"

#include "Helpers/MutationTestBed.h"

#include <gtest/gtest.h>

using namespace mull;
using namespace mull_test;
using namespace llvm;

static const std::string testCode = std::string(R"(///
int add(int a) {
  return !a;
}
)");

TEST(RemoveNegation, End_2_End) {
  mull::cxx::RemoveNegation mutator;

  MutationTestBed mutationTestBed;

  std::unique_ptr<MutationArtefact> artefact = mutationTestBed.generate(testCode, mutator);

  /// 1. AST Assertions
  LineColumnHash locationHash = lineColumnHash(3, 10);

  SingleASTUnitMutations singleUnitMutations = artefact->getASTMutations();
  ASSERT_EQ(singleUnitMutations.size(), 1U);

  SingleFileMutations &singleFileMutations = singleUnitMutations["/in-memory-file.cc"];
  ASSERT_EQ(singleFileMutations.count(MutatorKind::CXX_RemoveNegation), 1U);

  ASSERT_EQ(singleFileMutations[MutatorKind::CXX_RemoveNegation].size(), 1U);
  ASSERT_EQ(singleFileMutations[MutatorKind::CXX_RemoveNegation].count(locationHash), 1U);

  /// 2. IR and Junk Detection Assertions
  std::vector<MutationPoint *> nonJunkMutationPoints = artefact->getNonJunkMutationPoints();
  std::vector<MutationPoint *> junkMutationPoints = artefact->getJunkMutationPoints();

  ASSERT_EQ(nonJunkMutationPoints.size(), 1);
  ASSERT_EQ(junkMutationPoints.size(), 0);

  MutationPoint &mutationPoint = *nonJunkMutationPoints.at(0);

  auto const dumpRegex = std::regex("Mutation Point: cxx_remove_negation /in-memory-file.cc:3:10");
  ASSERT_TRUE(std::regex_search(mutationPoint.dump(), dumpRegex));
}
