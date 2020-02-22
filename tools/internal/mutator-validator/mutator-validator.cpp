#include <cassert>
#include <utility>
#include <vector>

#include <llvm/IR/Verifier.h>

#include <mull/BitcodeLoader.h>
#include <mull/Config/Configuration.h>
#include <mull/Diagnostics/Diagnostics.h>
#include <mull/MutationsFinder.h>
#include <mull/Mutators/MutatorsFactory.h>
#include <mull/Parallelization/Parallelization.h>
#include <mull/Program/Program.h>
#include <mull/Toolchain/Toolchain.h>

int main(int argc, char **argv) {
  assert(argc == 3 && "Expect mutator name and path to a bitcode file");
  mull::Diagnostics diagnostics;
  std::vector<std::string> mutatorGroups({ argv[1] });
  const char *bitcodePath = argv[2];

  mull::BitcodeLoader loader;
  llvm::LLVMContext context;
  std::vector<std::unique_ptr<mull::Bitcode>> bitcode;
  bitcode.push_back(std::move(loader.loadBitcodeAtPath(bitcodePath, context, diagnostics)));
  mull::Program program({}, {}, std::move(bitcode));

  mull::MutatorsFactory factory(diagnostics);
  auto mutators = factory.mutators(mutatorGroups);

  mull::Configuration configuration;
  mull::MutationsFinder finder(std::move(mutators), configuration);

  std::vector<mull::FunctionUnderTest> functionsUnderTest;
  for (auto &bc : program.bitcode()) {
    for (auto &function : bc->getModule()->functions()) {
      functionsUnderTest.emplace_back(&function, nullptr, 1);
    }
  }

  auto mutants = finder.getMutationPoints(diagnostics, program, functionsUnderTest);

  printf("Found %lu mutants\n", mutants.size());

  mull::SingleTaskExecutor prepareMutants(diagnostics, "Preparing mutants", [&] {
    for (auto &bc : program.bitcode()) {
      mull::Bitcode &bitcode = *bc;
      std::vector<std::string> mutatedFunctions;
      mull::CloneMutatedFunctionsTask::cloneFunctions(bitcode, mutatedFunctions);
      mull::DeleteOriginalFunctionsTask::deleteFunctions(bitcode);
      mull::InsertMutationTrampolinesTask::insertTrampolines(bitcode);
    }
  });
  prepareMutants.execute();

  mull::SingleTaskExecutor applyMutants(diagnostics, "Applying mutants", [&] {
    for (auto &mutant : mutants) {
      mutant->applyMutation();
    }
  });
  applyMutants.execute();

  mull::Toolchain toolchain(diagnostics, configuration);
  mull::SingleTaskExecutor compileMutants(diagnostics, "Compiling mutants", [&] {
    for (auto &bc : program.bitcode()) {
      assert(!llvm::verifyModule(*bc->getModule(), &llvm::errs()));
      toolchain.compiler().compileModule(bc->getModule(), toolchain.targetMachine());
    }
  });
  compileMutants.execute();

  return 0;
}
