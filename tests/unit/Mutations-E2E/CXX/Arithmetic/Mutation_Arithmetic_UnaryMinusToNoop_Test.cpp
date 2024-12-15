#include "FixturePaths.h"

#include "mull/Bitcode.h"
#include "mull/MutationPoint.h"
#include "mull/Mutators/CXX/ArithmeticMutators.h"

#include "Helpers/MutationTestBed.h"

#include <gtest/gtest.h>

#include <regex>

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

  std::vector<MutationPoint *> nonJunkMutationPoints = artefact->getNonJunkMutationPoints();
  std::vector<MutationPoint *> junkMutationPoints = artefact->getJunkMutationPoints();

  ASSERT_EQ(nonJunkMutationPoints.size(), 1);
  ASSERT_EQ(junkMutationPoints.size(), 0);

  MutationPoint &mutationPoint = *nonJunkMutationPoints.at(0);

  auto const dumpRegex = std::regex("Mutation Point: cxx_minus_to_noop /in-memory-file.cc:3:10");
  ASSERT_TRUE(std::regex_search(mutationPoint.dump(), dumpRegex));
}
