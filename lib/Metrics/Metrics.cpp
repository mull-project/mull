#include "Metrics/Metrics.h"
#include <iostream>
#include <numeric>

using namespace mull;

MetricsMeasure::Precision currentTimestamp() {
  using namespace std::chrono;
  using clock = system_clock;
  return duration_cast<MetricsMeasure::Precision>(clock::now().time_since_epoch());
}

template <class Key>
MetricsMeasure::Duration accumulate_duration(const std::map<Key, MetricsMeasure> &metrics) {
  MetricsMeasure::Duration total(0);
  for (auto &pair: metrics) {
    total += pair.second.duration();
  }
  return total;
}

template <class Key>
MetricsMeasure::Duration average_duration(const std::map<Key, MetricsMeasure> &metrics) {
  if (metrics.empty()) {
    return MetricsMeasure::Duration(0);
  }

  auto avg = accumulate_duration(metrics) / metrics.size();
  return MetricsMeasure::Duration(avg);
}

MetricsMeasure::MetricsMeasure() : begin(0), end(0) {}

void MetricsMeasure::start() {
  begin = currentTimestamp();
}

void MetricsMeasure::finish() {
  end = currentTimestamp();
}

MetricsMeasure::Duration MetricsMeasure::duration() const {
  return (end - begin).count();
}

const char *MetricsMeasure::precision() {
  return "ms";
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

void Metrics::beginInstrumentedCompilation() {
  instrumentedCompilation.begin = currentTimestamp();
}

void Metrics::endInstrumentedCompilation() {
  instrumentedCompilation.end = currentTimestamp();
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

void Metrics::beginOriginalTestExecution() {
  originalTestsExecution.begin = currentTimestamp();
}

void Metrics::endOriginalTestExecution() {
  originalTestsExecution.end = currentTimestamp();
}

void Metrics::beginMutantsExecution() {
  mutantsExecution.begin = currentTimestamp();
}

void Metrics::endMutantsExecution() {
  mutantsExecution.end = currentTimestamp();
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

void Metrics::dump() const {
  using namespace std;

  MetricsMeasure::Duration totalMutantRunTime(0);
  for (auto &pair: mutantRuns) {
    totalMutantRunTime += accumulate_duration(pair.second);
  }

  cout << "Execution Metrics:" << endl;
  cout << endl;

  cout << "Total run time: ................... " << runTime.duration() << MetricsMeasure::precision() << endl;
  // cout << "Saving result: .................... " << reportResult.duration() << MetricsMeasure::precision() << endl;
  cout << endl;

  cout << "Find tests: ....................... " << findTests.duration() << MetricsMeasure::precision() << endl;
  cout << "Find mutations: ................... " << accumulate_duration(findMutations) << MetricsMeasure::precision() << endl;
  cout << endl;

  cout << "Load modules: ..................... " << loadModules.duration() << MetricsMeasure::precision() << endl;
  cout << "Load object files: ................ " << loadPrecompiledObjectFiles.duration() << MetricsMeasure::precision() << endl;
  cout << "Load dylibs: ...................... " << loadDynamicLibraries.duration() << MetricsMeasure::precision() << endl;
  cout << endl;

  cout << "Load original program: ............ " << loadOriginalProgram.duration() << MetricsMeasure::precision() << endl;
  cout << "Load mutant (avg): ................ " << average_duration(loadMutant) << MetricsMeasure::precision() << endl;
  cout << "Load mutant (total): .............. " << accumulate_duration(loadMutant) << MetricsMeasure::precision() << endl;
  cout << endl;

  cout << "Original compilation (total): ..... " << accumulate_duration(originalModuleCompilation) << MetricsMeasure::precision() << endl;
  cout << "Instrumented compilation (total): . " << instrumentedCompilation.duration() << MetricsMeasure::precision() << endl;
  cout << "Mutant compilation (total): ....... " << accumulate_duration(compileMutant) << MetricsMeasure::precision() << endl;
  cout << endl;

  cout << "Original compilation (avg): ....... " << average_duration(originalModuleCompilation) << MetricsMeasure::precision() << endl;
  cout << "Instrumented compilation (avg): ... " << average_duration(instrumentedModuleCompilation) << MetricsMeasure::precision() << endl;
  cout << "Mutant compilation (avg): ......... " << average_duration(compileMutant) << MetricsMeasure::precision() << endl;
  cout << endl;

  cout << "Tests run time (total): ........... " << originalTestsExecution.duration() << MetricsMeasure::precision() << endl;
  cout << "Mutants run time (total): ......... " << mutantsExecution.duration() << MetricsMeasure::precision() << endl;
  cout << "Tests run time (avg): ............. " << average_duration(runOriginalTest) << MetricsMeasure::precision() << endl;
  cout << "Mutants run time (avg): ........... " << totalMutantRunTime / (mutantRuns.size() ? mutantRuns.size() : 1) << MetricsMeasure::precision() << endl;
  cout << endl;
}

