#include "FixturePaths.h"

#include "mull/Bitcode.h"
#include "mull/MutationPoint.h"
#include "mull/Mutators/CXX/NumberMutators.h"

#include <clang/Frontend/ASTUnit.h>
#include <clang/Tooling/Tooling.h>
#include <llvm/IR/Module.h>
#include <regex>

#include "Helpers/MutationTestBed.h"

#include <gtest/gtest.h>

using namespace mull;
using namespace mull::cxx;
using namespace mull_test;
using namespace llvm;

static FilePathFilter nullPathFilter;

static const std::string testCode = std::string(R"(///
int foo() {
  int var;
  var = 0;
  return var;
}
)");

TEST(Mutation_ConstAssignment_AssignConst, End_2_End) {
  NumberAssignConst mutator;

  MutationTestBed mutationTestBed;

  std::unique_ptr<MutationArtefact> artefact = mutationTestBed.generate(testCode, mutator);

  std::vector<MutationPoint *> nonJunkMutationPoints = artefact->getNonJunkMutationPoints();
  std::vector<MutationPoint *> junkMutationPoints = artefact->getJunkMutationPoints();

  ASSERT_EQ(junkMutationPoints.size(), 0);
  ASSERT_EQ(nonJunkMutationPoints.size(), 1);

  {
    MutationPoint &mutationPoint = *nonJunkMutationPoints.at(0);

    auto const dumpRegex =
      std::regex("Mutation Point: cxx_assign_const /in-memory-file.cc:4:7");
    ASSERT_TRUE(std::regex_search(mutationPoint.dump(), dumpRegex));
  }
}
