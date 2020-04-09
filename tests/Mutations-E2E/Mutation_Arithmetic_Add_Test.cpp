#include "FixturePaths.h"

#include "mull/AST/ASTVisitor.h"
#include "mull/Bitcode.h"
#include "mull/JunkDetection/CXX/ASTStorage.h"
#include "mull/JunkDetection/CXX/CXXJunkDetector.h"
#include "mull/MutationPoint.h"
#include "mull/Mutators/CXX/ArithmeticMutators.h"
#include "mull/ReachableFunction.h"

#include "Helpers/InMemoryFixture.h"

#include <clang/Frontend/ASTUnit.h>
#include <clang/Tooling/Tooling.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/LLVMContext.h>

#include <gtest/gtest.h>

using namespace mull;
using namespace llvm;

static FilePathFilter nullPathFilter;

static const std::string testCode = std::string(R"(///
int add(int a, int b) {
  return a + b;
}
)");

mull_test::InMemoryFixture fixture;

TEST(Mutation_Arithmetic_AddToSub, 01_AST) {
  Diagnostics diagnostics;
  diagnostics.enableDebugMode();

  ASTStorage storage(diagnostics, "", "", {});

  std::unique_ptr<clang::ASTUnit> astUnit = fixture.createAST(testCode);
  assert(astUnit);

  ThreadSafeASTUnit threadSafeAstUnit(std::move(astUnit));

  SingleASTUnitMutations singleUnitMutations;
  MutatorKindSet mutatorKindSet = MutatorKindSet::create({ MutatorKind::CXX_AddToSub });
  ASTVisitor astVisitor(
      diagnostics, threadSafeAstUnit, singleUnitMutations, nullPathFilter, mutatorKindSet);

  astVisitor.traverse();

  LineColumnHash locationHash = lineColumnHash(3, 12);

  ASSERT_EQ(singleUnitMutations.size(), 1U);
  ASSERT_EQ(singleUnitMutations.count(MutatorKind::CXX_AddToSub), 1U);

  ASSERT_EQ(singleUnitMutations[MutatorKind::CXX_AddToSub].size(), 1U);
  ASSERT_EQ(singleUnitMutations[MutatorKind::CXX_AddToSub].count(locationHash), 1U);
}

TEST(Mutation_Arithmetic_AddToSub, 02_IR_and_Junk) {
  llvm::LLVMContext context;

  std::unique_ptr<clang::ASTUnit> astUnit = fixture.createAST(testCode);
  std::unique_ptr<llvm::Module> module = fixture.createIR(testCode, context);
  std::unique_ptr<MemoryBuffer> testCodeAsBuffer = fixture.createMemoryBuffer(testCode);

  /// 1. Finding mutations in IR
  Bitcode bitcode(std::move(module), std::move(testCodeAsBuffer), "IRRELEVANT");

  mull::cxx::AddToSub mutator;

  Diagnostics diagnostics;
  std::vector<MutationPoint *> points;
  for (auto &function : bitcode.getModule()->functions()) {
    FunctionUnderTest functionUnderTest(&function, nullptr, 0);
    functionUnderTest.selectInstructions({});
    auto mutants = mutator.getMutations(&bitcode, functionUnderTest);
    std::copy(mutants.begin(), mutants.end(), std::back_inserter(points));
  }
  ASSERT_EQ(points.size(), 1);

  /// 2. Junk Detection
  std::unique_ptr<ThreadSafeASTUnit> threadSafeAstUnit(new ThreadSafeASTUnit(std::move(astUnit)));

  ASTStorage astStorage(diagnostics, "", "", {});
  astStorage.setAST("/in-memory-file.cc", std::move(threadSafeAstUnit));

  CXXJunkDetector detector(astStorage);

  std::vector<MutationPoint *> nonJunkMutationPoints;
  std::vector<MutationPoint *> junkMutationPoints;
  for (auto point : points) {
    if (!detector.isJunk(point)) {
      nonJunkMutationPoints.push_back(point);
    } else {
      junkMutationPoints.push_back(point);
    }
  }

  ASSERT_EQ(nonJunkMutationPoints.size(), 1);
  ASSERT_EQ(junkMutationPoints.size(), 0);

  MutationPoint &mutationPoint = *nonJunkMutationPoints.at(0);

  auto const dumpRegex = std::regex("Mutation Point: cxx_add_to_sub /in-memory-file.cc:3:12");
  ASSERT_TRUE(std::regex_search(mutationPoint.dump(), dumpRegex));
}
