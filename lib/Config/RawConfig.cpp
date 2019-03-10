#include "Logger.h"
#include "Config/RawConfig.h"
#include "Config/Configuration.h"

#include <llvm/Support/YAMLTraits.h>
#include <llvm/Support/FileSystem.h>
#include <fstream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

int MullDefaultTimeoutMilliseconds = 3000;

using namespace mull;

JunkDetectionConfig::JunkDetectionConfig()
: toggle(JunkDetectionToggle::Disabled),
  detectorName(""),
  cxxCompilationDatabasePath(""),
  cxxCompilationFlags("") {}

JunkDetectionConfig JunkDetectionConfig::enabled() {
  JunkDetectionConfig config;
  config.toggle = JunkDetectionToggle::Enabled;
  return config;
}

JunkDetectionConfig JunkDetectionConfig::disabled() {
  JunkDetectionConfig config;
  config.toggle = JunkDetectionToggle::Disabled;
  return config;
}

bool JunkDetectionConfig::isEnabled() const {
  return toggle == JunkDetectionToggle::Enabled;
}

std::string RawConfig::forkToString(Fork fork) {
  switch (fork) {
    case Fork::Enabled:
      return "enabled";
      break;

    case Fork::Disabled:
      return "disabled";
      break;
  }
}


std::string RawConfig::dryRunToString(DryRunMode dryRun) {
  switch (dryRun) {
    case DryRunMode::Enabled:
      return "enabled";
      break;

    case DryRunMode::Disabled:
      return "disabled";
      break;
  }
}

std::string RawConfig::failFastToString(FailFastMode failFast) {
  switch (failFast) {
    case FailFastMode::Enabled:
      return "enabled";
      break;

    case FailFastMode::Disabled:
      return "disabled";
      break;
  }
}

std::string RawConfig::cachingToString(UseCache caching) {
  switch (caching) {
    case UseCache::Yes:
      return "yes";
      break;

    case UseCache::No:
      return "no";
      break;
  }
}
std::string RawConfig::emitDebugInfoToString(EmitDebugInfo emitDebugInfo) {
  switch (emitDebugInfo) {
    case EmitDebugInfo::Yes:
      return "yes";
      break;

    case EmitDebugInfo::No:
      return "no";
      break;
  }
}

// Constructor initializes defaults.
// TODO: Refactoring into constants.
RawConfig::RawConfig() :
  bitcodeFileList(""),
  projectName(""),
  testFramework("GoogleTest"),
  mutators(
                    // Yaml::Traits stops reading mutators from config.yaml
                    // if these 3 default mutators are set here (BUG?).
                    // So leaving out the empty ()
                    // {
                    //   MathAddMutator::ID,
                    //   NegateConditionMutator::ID,
                    //   RemoveVoidFunctionMutator::ID
                    // }
                    ),
  reporters(),
  dynamicLibraryFileList(),
  objectFileList(),
  tests(),
  excludeLocations(),
  customTests(),
  fork(Fork::Enabled),
  dryRun(DryRunMode::Disabled),
  failFast(FailFastMode::Disabled),
  caching(UseCache::No),
  emitDebugInfo(EmitDebugInfo::No),
  diagnostics(Diagnostics::None),
  timeout(MullDefaultTimeoutMilliseconds),
  maxDistance(128),
  cacheDirectory("/tmp/mull_cache"),
  junkDetection(),
  parallelizationConfig()
{}

RawConfig::RawConfig(const std::string &bitcodeFileList,
               const std::string &project,
               const std::string &testFramework,
               const std::vector<std::string> mutators,
               const std::vector<std::string> reporters,
               const std::string &dynamicLibraryFileList,
               const std::string &objectFileList,
               const std::vector<std::string> tests,
               const std::vector<std::string> excludeLocations,
               const std::vector<CustomTestDefinition> definitions,
               Fork fork,
               DryRunMode dryRun,
               FailFastMode failFast,
               UseCache cache,
               EmitDebugInfo debugInfo,
               Diagnostics diagnostics,
               int timeout,
               int distance,
               const std::string &cacheDir,
               JunkDetectionConfig junkDetection,
               ParallelizationConfig parallelizationConfig) :
bitcodeFileList(bitcodeFileList),
projectName(project),
testFramework(testFramework),
mutators(mutators),
reporters(reporters),
dynamicLibraryFileList(dynamicLibraryFileList),
objectFileList(objectFileList),
tests(tests),
excludeLocations(excludeLocations),
customTests(definitions),
fork(fork),
dryRun(dryRun),
failFast(failFast),
caching(cache),
emitDebugInfo(debugInfo),
diagnostics(diagnostics),
timeout(timeout),
maxDistance(distance),
cacheDirectory(cacheDir),
junkDetection(std::move(junkDetection)),
parallelizationConfig(parallelizationConfig)
{
}

const std::string &RawConfig::getBitcodeFileList() const {
  return bitcodeFileList;
}

std::vector<std::string> RawConfig::getBitcodePaths() const {
  std::vector<std::string> bitcodePaths;

  std::ifstream ifs(bitcodeFileList);

  for (std::string path; getline(ifs, path); ) {
    if (path.at(0) == '#') {
      continue;
    }

    bitcodePaths.push_back(path);
  }

  return bitcodePaths;
}

const std::string &RawConfig::getProjectName() const {
  return projectName;
}

const std::string &RawConfig::getTestFramework() const {
  return testFramework;
}

const std::string &RawConfig::getDynamicLibraries() const {
  return dynamicLibraryFileList;
}

std::vector<std::string> RawConfig::getDynamicLibrariesPaths() const {
  std::vector<std::string> dynamicLibrariesPaths;

  std::ifstream ifs(dynamicLibraryFileList);

  for (std::string path; getline(ifs, path); ) {
    if (path.at(0) == '#') {
      continue;
    }

    dynamicLibrariesPaths.push_back(path);
  }

  return dynamicLibrariesPaths;
}

const std::string &RawConfig::getObjectFileList() const {
  return objectFileList;
}

std::vector<std::string> RawConfig::getObjectFilesPaths() const {
  std::vector<std::string> objectFilesPaths;

  std::ifstream ifs(objectFileList);

  for (std::string path; getline(ifs, path); ) {
    if (path.at(0) == '#') {
      continue;
    }

    objectFilesPaths.push_back(path);
  }

  return objectFilesPaths;
}

const std::vector<std::string> &RawConfig::getMutators() const {
  return mutators;
}

const std::vector<std::string> &RawConfig::getReporters() const {
  return reporters;
}

const std::vector<std::string> &RawConfig::getTests() const {
  return tests;
}

const std::vector<std::string> &RawConfig::getExcludeLocations() const {
  return excludeLocations;
}

const std::vector<CustomTestDefinition> &RawConfig::getCustomTests() const {
  return customTests;
}

void RawConfig::addCustomTest(CustomTestDefinition customTest) {
  customTests.push_back(customTest);
}

bool RawConfig::forkEnabled() const {
  return fork == Fork::Enabled;
}

int RawConfig::getTimeout() const {
  return timeout;
}

bool RawConfig::cachingEnabled() const {
  return caching == UseCache::Yes;
}

bool RawConfig::dryRunModeEnabled() const {
  return dryRun == DryRunMode::Enabled;
}

bool RawConfig::failFastModeEnabled() const {
  return failFast == FailFastMode::Enabled;
}

bool RawConfig::shouldEmitDebugInfo() const {
  return emitDebugInfo == EmitDebugInfo::Yes;
}

bool RawConfig::junkDetectionEnabled() const {
  return junkDetection.isEnabled();
}

JunkDetectionConfig &RawConfig::junkDetectionConfig() {
  return junkDetection;
}

Diagnostics RawConfig::getDiagnostics() const {
  return diagnostics;
}

int RawConfig::getMaxDistance() const {
  return maxDistance;
}

std::string RawConfig::getCacheDirectory() const {
  return cacheDirectory;
}

void RawConfig::dump() const {
  Logger::debug() << "RawConfig>\n"
  << "\t" << "bitcode_file_list: " << bitcodeFileList << '\n'
  << "\t" << "dynamic_library_file_list: " << dynamicLibraryFileList << '\n'
  << "\t" << "object_file_list: " << objectFileList << '\n'
  << "\t" << "project_name: " << getProjectName() << '\n'
  << "\t" << "test_framework: " << getTestFramework() << '\n'
  << "\t" << "distance: " << getMaxDistance() << '\n'
  << "\t" << "dry_run: " << dryRunToString(dryRun) << '\n'
  << "\t" << "fail_fast: " << failFastToString(failFast) << '\n'
  << "\t" << "fork: " << forkToString(fork) << '\n'
  << "\t" << "use_cache: " << cachingToString(caching) << '\n'
  << "\t" << "junk_detection: " << (junkDetectionEnabled() ? "enabled" : "disabled" ) << '\n'
  << "\t" << "diagnostics: " << diagnosticsToString(diagnostics) << '\n'
  << "\t" << "emit_debug_info: " << emitDebugInfoToString(emitDebugInfo) << '\n';

  if (!mutators.empty()) {
    Logger::debug() << "\t" << "mutators: " << '\n';

    for (const auto &mutator : mutators) {
      Logger::debug() << "\t- " << mutator << '\n';
    }
  }

  if (!getTests().empty()) {
    Logger::debug() << "\t" << "tests: " << '\n';

    for (const auto &test : getTests()) {
      Logger::debug() << "\t- " << test << '\n';
    }
  }

  if (!getExcludeLocations().empty()) {
    Logger::debug() << "\t" << "exclude_locations: " << '\n';

    for (const auto &excludeLocation : getExcludeLocations()) {
      Logger::debug() << "\t- " << excludeLocation << '\n';
    }
  }
}

std::vector<std::string> RawConfig::validate() {
  std::vector<std::string> errors;

  if (bitcodeFileList.empty()) {
    std::string error = "bitcode_file_list parameter is not specified.";
    errors.push_back(error);
    return errors;
  }

  if (!llvm::sys::fs::exists(bitcodeFileList)) {
    std::stringstream error;

    error << "bitcode_file_list parameter points to a non-existing file: "
          << bitcodeFileList;

    errors.push_back(error.str());

  }

  if (!dynamicLibraryFileList.empty()) {
    if (!llvm::sys::fs::exists(dynamicLibraryFileList)) {
      std::stringstream error;

      error << "dynamic_library_file_list parameter points to a non-existing file: "
            << dynamicLibraryFileList;

      errors.push_back(error.str());
    }
  }

  if (!objectFileList.empty()) {
    if (!llvm::sys::fs::exists(objectFileList)) {
      std::stringstream error;

      error << "object_file_list parameter points to a non-existing file: "
            << objectFileList;

      errors.push_back(error.str());
    }
  }

  return errors;
}

const ParallelizationConfig RawConfig::parallelization() const {
  return parallelizationConfig;
}

void RawConfig::normalizeParallelizationConfig() {
  parallelizationConfig.normalize();
}

