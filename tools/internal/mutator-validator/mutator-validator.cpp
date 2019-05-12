#include <cassert>
#include <utility>
#include <vector>

#include <llvm/IR/Verifier.h>

#include <mull/Config/Configuration.h>
#include <mull/Filter.h>
#include <mull/ModuleLoader.h>
#include <mull/MutationsFinder.h>
#include <mull/Mutators/MutatorsFactory.h>
#include <mull/Parallelization/TaskExecutor.h>
#include <mull/Program/Program.h>
#include <mull/Toolchain/Toolchain.h>

int main(int argc, char **argv) {
  assert(argc == 3 && "Expect mutator name and path to a bitcode file");

  std::vector<std::string> mutatorGroups({argv[1]});
  const char *bitcodePath = argv[2];

  mull::ModuleLoader loader;
  llvm::LLVMContext context;
  std::vector<std::unique_ptr<mull::MullModule>> modules;
  modules.push_back(std::move(loader.loadModuleAtPath(bitcodePath, context)));
  mull::Program program({}, {}, std::move(modules));

  mull::MutatorsFactory factory;
  auto mutators = factory.mutators(mutatorGroups);

  mull::Configuration configuration;
  mull::MutationsFinder finder(std::move(mutators), configuration);

  std::vector<mull::MergedTestee> testees;
  for (auto &module : program.modules()) {
    for (auto &function : module->getModule()->functions()) {
      testees.emplace_back(&function, nullptr, 1);
    }
  }

  mull::Filter filter;

  auto mutants = finder.getMutationPoints(program, testees, filter);

  printf("Found %lu mutants\n", mutants.size());

  mull::SingleTaskExecutor prepareMutants("Preparing mutants", [&] {
    for (auto &module : program.modules()) {
      module->prepareMutations();
    }
  });
  prepareMutants.execute();

  mull::SingleTaskExecutor applyMutants("Applying mutants", [&] {
    for (auto &mutant : mutants) {
      mutant->applyMutation();
    }
  });
  applyMutants.execute();

  mull::Toolchain toolchain(configuration);
  mull::SingleTaskExecutor compileMutants("Compiling mutants", [&] {
    for (auto &module : program.modules()) {
      assert(!llvm::verifyModule(*module->getModule(), &llvm::errs()));
      toolchain.compiler().compileModule(module->getModule(),
                                         toolchain.targetMachine());
    }
  });
  compileMutants.execute();

  return 0;
}
