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
int add(int a, int b) {
  return a + b;
}
)");

TEST(Mutation_Arithmetic_AddToSub, End_2_End) {
  mull::cxx::AddToSub mutator;

  MutationTestBed mutationTestBed;

  std::unique_ptr<MutationArtefact> artefact = mutationTestBed.generate(testCode, mutator);

  std::vector<MutationPoint *> nonJunkMutationPoints = artefact->getNonJunkMutationPoints();
  std::vector<MutationPoint *> junkMutationPoints = artefact->getJunkMutationPoints();

  ASSERT_EQ(nonJunkMutationPoints.size(), 1);
  ASSERT_EQ(junkMutationPoints.size(), 0);

  MutationPoint &mutationPoint = *nonJunkMutationPoints.at(0);

  auto const dumpRegex = std::regex("Mutation Point: cxx_add_to_sub /in-memory-file.cc:3:12");
  ASSERT_TRUE(std::regex_search(mutationPoint.dump(), dumpRegex));
}
