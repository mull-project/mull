#pragma once

#include <chrono>
#include <map>

namespace llvm {
  class Module;
}

namespace mull {

class Test;
class MutationPoint;

struct MetricsMeasure {
  using Precision = std::chrono::milliseconds;
  using Duration = std::chrono::milliseconds::rep;

  Precision begin;
  Precision end;

  MetricsMeasure() : begin(0), end(0) {}
  Duration duration() {
    return (end - begin).count();
  }
};

class Metrics {
public:
  void beginLoadModules();
  void endLoadModules();

  void beginCompileOriginalModule(const llvm::Module *module);
  void endCompileOriginalModule(const llvm::Module *module);

  void beginCompileInstrumentedModule(const llvm::Module *module);
  void endCompileInstrumentedModule(const llvm::Module *module);

  void beginLoadPrecompiledObjectFiles();
  void endLoadPrecompiledObjectFiles();

  void beginFindTests();
  void endFindTests();

  void beginLoadDynamicLibraries();
  void endLoadDynamicLibraries();

  void beginLoadOriginalProgram();
  void endLoadOriginalProgram();

  void beginRunOriginalTest(const Test *test);
  void endRunOriginalTest(const Test *test);

  void beginFindMutationsForTest(const Test *test);
  void endFindMutationsForTest(const Test *test);

  void beginCompileMutant(const MutationPoint *mutant);
  void endCompileMutant(const MutationPoint *mutant);

  void beginLoadMutatedProgram(const MutationPoint *mutant);
  void endLoadMutatedProgram(const MutationPoint *mutant);

  void beginRunMutant(const MutationPoint *mutant, const Test *test);
  void endRunMutant(const MutationPoint *mutant, const Test *test);

  void beginRun();
  void endRun();

  void beginReportResult();
  void endReportResult();

  void dump();
private:
  MetricsMeasure loadModules;
  MetricsMeasure loadPrecompiledObjectFiles;
  MetricsMeasure loadDynamicLibraries;
  MetricsMeasure loadOriginalProgram;
  MetricsMeasure findTests;
  MetricsMeasure runTime;
  MetricsMeasure reportResult;

  std::map<const llvm::Module *, MetricsMeasure> originalModuleCompilation;
  std::map<const llvm::Module *, MetricsMeasure> instrumentedModuleCompilation;

  std::map<const Test *, MetricsMeasure> runOriginalTest;
  std::map<const Test *, MetricsMeasure> findMutations;

  std::map<const MutationPoint *, MetricsMeasure> compileMutant;
  std::map<const MutationPoint *, MetricsMeasure> loadMutant;

  std::map<const MutationPoint *, std::map<const Test *, MetricsMeasure>> mutantRuns;
};

}
