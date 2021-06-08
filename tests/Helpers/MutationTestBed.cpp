#include "MutationTestBed.h"

#include "mull/Bitcode.h"
#include "mull/Diagnostics/Diagnostics.h"
#include "mull/FunctionUnderTest.h"
#include "mull/JunkDetection/CXX/ASTStorage.h"
#include "mull/JunkDetection/CXX/CXXJunkDetector.h"
#include "mull/Mutators/Mutator.h"

using namespace mull;
using namespace mull_test;

MutationTestBed::MutationTestBed() : fixtureGenerator(), nullPathFilter() {
  diagnostics.enableDebugMode();
}

std::unique_ptr<MutationArtefact> MutationTestBed::generate(const std::string &sourceCode,
                                                            mull::Mutator &mutator) {
  std::unique_ptr<llvm::LLVMContext> context(new llvm::LLVMContext);

  std::unique_ptr<clang::ASTUnit> astUnit = fixtureGenerator.createAST(sourceCode);
  std::unique_ptr<llvm::Module> module = fixtureGenerator.createIR(sourceCode, *context);
  std::unique_ptr<llvm::MemoryBuffer> testCodeAsBuffer =
      fixtureGenerator.createMemoryBuffer(sourceCode);

  std::unique_ptr<ThreadSafeASTUnit> threadSafeAstUnit(new ThreadSafeASTUnit(std::move(astUnit)));
  ASTStorage storage(diagnostics, "", "", {});

  /// 1. Finding mutations in IR

  std::unique_ptr<Bitcode> bitcode(new Bitcode(std::move(context), std::move(module)));

  std::vector<MutationPoint *> points;
  for (auto &function : bitcode->getModule()->functions()) {
    FunctionUnderTest functionUnderTest(&function, bitcode.get());
    functionUnderTest.selectInstructions({});
    auto mutants = mutator.getMutations(bitcode.get(), functionUnderTest);
    std::copy(mutants.begin(), mutants.end(), std::back_inserter(points));
  }

  /// 2. Junk Detection

  ASTStorage astStorage(diagnostics, "", "", {});
  astStorage.setAST("/in-memory-file.cc", std::move(threadSafeAstUnit));

  CXXJunkDetector detector(diagnostics, astStorage);

  std::vector<MutationPoint *> nonJunkMutationPoints;
  std::vector<MutationPoint *> junkMutationPoints;
  for (auto point : points) {
    if (!detector.isJunk(point)) {
      nonJunkMutationPoints.push_back(point);
    } else {
      junkMutationPoints.push_back(point);
    }
  }

  std::unique_ptr<MutationArtefact> artefact(new MutationArtefact(nonJunkMutationPoints,
                                                                  junkMutationPoints,
                                                                  std::move(context),
                                                                  std::move(bitcode)));

  return artefact;
}
