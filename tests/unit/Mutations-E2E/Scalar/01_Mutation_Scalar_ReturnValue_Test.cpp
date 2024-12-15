#include "FixturePaths.h"

#include "mull/Bitcode.h"
#include "mull/MutationPoint.h"
#include <mull/Mutators/ScalarValueMutator.h>

#include <clang/Frontend/ASTUnit.h>
#include <clang/Tooling/Tooling.h>
#include <llvm/IR/Module.h>

#include "Helpers/MutationTestBed.h"

#include <gtest/gtest.h>

#include <regex>

using namespace mull;
using namespace mull_test;
using namespace llvm;

static FilePathFilter nullPathFilter;

static const std::string testCode = std::string(R"(///
int foo() {
  return 0;
}
)");

TEST(Mutation_Scalar_Return_Value, End_2_End) {
  mull::ScalarValueMutator mutator;

  MutationTestBed mutationTestBed;

  std::unique_ptr<MutationArtefact> artefact = mutationTestBed.generate(testCode, mutator);

  std::vector<MutationPoint *> nonJunkMutationPoints = artefact->getNonJunkMutationPoints();
  std::vector<MutationPoint *> junkMutationPoints = artefact->getJunkMutationPoints();

  ASSERT_EQ(nonJunkMutationPoints.size(), 1);
  ASSERT_EQ(junkMutationPoints.size(), 0);

  {
    MutationPoint &mutationPoint = *nonJunkMutationPoints.at(0);

    auto const dumpRegex =
      std::regex("Mutation Point: scalar_value_mutator /in-memory-file.cc:3:3");
    ASSERT_TRUE(std::regex_search(mutationPoint.dump(), dumpRegex));
  }
}
