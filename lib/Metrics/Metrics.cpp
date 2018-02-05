#include "Metrics/Metrics.h"
#include <iostream>
#include <numeric>

using namespace mull;

MetricsMeasure::Precision currentTimestamp() {
  using namespace std::chrono;
  using clock = high_resolution_clock;
  return duration_cast<MetricsMeasure::Precision>(clock::now().time_since_epoch());
}

template <class Key>
MetricsMeasure::Duration accumulate_duration(std::map<Key, MetricsMeasure> &metrics) {
  MetricsMeasure::Duration total(0);
  for (auto &pair: metrics) {
    total += pair.second.duration();
  }
  return total;
}

void Metrics::beginLoadModules() {
  loadModules.begin = currentTimestamp();
}
void Metrics::endLoadModules() {
  loadModules.end = currentTimestamp();
}

void Metrics::beginCompileOriginalModule(const llvm::Module *module) {
  MetricsMeasure &measure = originalModuleCompilation[module];
  measure.begin = currentTimestamp();
}
void Metrics::endCompileOriginalModule(const llvm::Module *module) {
  MetricsMeasure &measure = originalModuleCompilation[module];
  measure.end = currentTimestamp();
}

void Metrics::beginCompileInstrumentedModule(const llvm::Module *module) {
  MetricsMeasure &measure = instrumentedModuleCompilation[module];
  measure.begin = currentTimestamp();
}
void Metrics::endCompileInstrumentedModule(const llvm::Module *module) {
  MetricsMeasure &measure = instrumentedModuleCompilation[module];
  measure.end = currentTimestamp();
}

void Metrics::beginLoadPrecompiledObjectFiles() {
  loadPrecompiledObjectFiles.begin = currentTimestamp();
}
void Metrics::endLoadPrecompiledObjectFiles() {
  loadPrecompiledObjectFiles.end = currentTimestamp();
}

void Metrics::beginFindTests() {
  findTests.begin = currentTimestamp();
}
void Metrics::endFindTests() {
  findTests.end = currentTimestamp();
}

void Metrics::beginLoadDynamicLibraries() {
  loadDynamicLibraries.begin = currentTimestamp();
}
void Metrics::endLoadDynamicLibraries() {
  loadDynamicLibraries.end = currentTimestamp();
}

void Metrics::beginLoadOriginalProgram() {
  loadOriginalProgram.begin = currentTimestamp();
}
void Metrics::endLoadOriginalProgram() {
  loadOriginalProgram.end = currentTimestamp();
}

void Metrics::beginRunOriginalTest(const Test *test) {
  MetricsMeasure &measure = runOriginalTest[test];
  measure.begin = currentTimestamp();
}
void Metrics::endRunOriginalTest(const Test *test) {
  MetricsMeasure &measure = runOriginalTest[test];
  measure.end = currentTimestamp();
}

void Metrics::beginFindMutationsForTest(const Test *test) {
  MetricsMeasure &measure = findMutations[test];
  measure.begin = currentTimestamp();
}
void Metrics::endFindMutationsForTest(const Test *test) {
  MetricsMeasure &measure = findMutations[test];
  measure.end = currentTimestamp();
}

void Metrics::beginCompileMutant(const MutationPoint *mutant) {
  MetricsMeasure &measure = compileMutant[mutant];
  measure.begin = currentTimestamp();
}
void Metrics::endCompileMutant(const MutationPoint *mutant) {
  MetricsMeasure &measure = compileMutant[mutant];
  measure.end = currentTimestamp();
}

void Metrics::beginLoadMutatedProgram(const MutationPoint *mutant) {
  MetricsMeasure &measure = loadMutant[mutant];
  measure.begin = currentTimestamp();
}
void Metrics::endLoadMutatedProgram(const MutationPoint *mutant) {
  MetricsMeasure &measure = loadMutant[mutant];
  measure.end = currentTimestamp();
}

void Metrics::beginRunMutant(const MutationPoint *mutant, const Test *test) {
  std::map<const Test *, MetricsMeasure> &testRun = mutantRuns[mutant];
  MetricsMeasure &measure = testRun[test];
  measure.begin = currentTimestamp();
}
void Metrics::endRunMutant(const MutationPoint *mutant, const Test *test) {
  std::map<const Test *, MetricsMeasure> &testRun = mutantRuns[mutant];
  MetricsMeasure &measure = testRun[test];
  measure.end = currentTimestamp();
}

void Metrics::beginRun() {
  runTime.begin = currentTimestamp();
}
void Metrics::endRun() {
  runTime.end = currentTimestamp();
}

void Metrics::beginReportResult() {
  reportResult.begin = currentTimestamp();
}
void Metrics::endReportResult() {
  reportResult.end = currentTimestamp();
}

void Metrics::dump() {
  using namespace std;

  MetricsMeasure::Duration totalMutantRunTime(0);
  for (auto &pair: mutantRuns) {
    totalMutantRunTime += accumulate_duration(pair.second);
  }

  cout << "Execution Metrics:" << endl;
  cout << endl;

  cout << "Total run time: ................... " << runTime.duration() << endl;
  cout << "Saving result: .................... " << reportResult.duration() << endl;
  cout << endl;

  cout << "Find tests: ....................... " << findTests.duration() << endl;
  cout << "Find mutations: ................... " << accumulate_duration(findMutations) << endl;
  cout << endl;

  cout << "Load modules: ..................... " << loadModules.duration() << endl;
  cout << "Load object files: ................ " << loadPrecompiledObjectFiles.duration() << endl;
  cout << "Load dylibs: ...................... " << loadDynamicLibraries.duration() << endl;
  cout << endl;

  cout << "Load original program: ............ " << loadOriginalProgram.duration() << endl;
  cout << "Load mutant (avg): ................ " << accumulate_duration(loadMutant) / loadMutant.size() << endl;
  cout << "Load mutant (total): .............. " << accumulate_duration(loadMutant) << endl;
  cout << endl;

  cout << "Original compilation (total): ..... " << accumulate_duration(originalModuleCompilation) << endl;
  cout << "Instrumented compilation (total): . " << accumulate_duration(instrumentedModuleCompilation) << endl;
  cout << "Mutant compilation (total): ....... " << accumulate_duration(compileMutant) << endl;
  cout << endl;

  cout << "Original compilation (avg): ....... " << accumulate_duration(originalModuleCompilation) / originalModuleCompilation.size() << endl;
  cout << "Instrumented compilation (avg): ... " << accumulate_duration(instrumentedModuleCompilation) / instrumentedModuleCompilation.size() << endl;
  cout << "Mutant compilation (avg): ......... " << accumulate_duration(compileMutant) / compileMutant.size() << endl;
  cout << endl;

  cout << "Tests run time (total): ........... " << accumulate_duration(runOriginalTest) << endl;
  cout << "Mutants run time (total): ......... " << totalMutantRunTime << endl;
  cout << "Tests run time (avg): ............. " << accumulate_duration(runOriginalTest) / runOriginalTest.size() << endl;
  cout << "Mutants run time (avg): ........... " << totalMutantRunTime / mutantRuns.size() << endl;
  cout << endl;
}
