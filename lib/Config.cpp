#include "Logger.h"
#include "Config.h"

#include <llvm/Support/YAMLTraits.h>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

int MullDefaultTimeoutMilliseconds = 3000;

using namespace mull;


CustomTestDefinition::CustomTestDefinition() {}

CustomTestDefinition::CustomTestDefinition(const std::string &name,
                                           const std::string &method,
                                           const std::string &program,
                                           const std::vector<std::string> arguments)
: testName(name), methodName(method), programName(program), callArguments(arguments) {}

JunkDetectionConfig::JunkDetectionConfig()
: toggle(JunkDetectionToggle::Disabled),
  detectorName(""),
  cxxCompDBDirectory(""),
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

std::string Config::forkToString(Fork fork) {
  switch (fork) {
    case Fork::Enabled:
      return "enabled";
      break;

    case Fork::Disabled:
      return "disabled";
      break;
  }
}


std::string Config::dryRunToString(DryRunMode dryRun) {
  switch (dryRun) {
    case DryRunMode::Enabled:
      return "enabled";
      break;

    case DryRunMode::Disabled:
      return "disabled";
      break;
  }
}

std::string Config::failFastToString(FailFastMode failFast) {
  switch (failFast) {
    case FailFastMode::Enabled:
      return "enabled";
      break;

    case FailFastMode::Disabled:
      return "disabled";
      break;
  }
}

std::string Config::cachingToString(UseCache caching) {
  switch (caching) {
    case UseCache::Yes:
      return "yes";
      break;

    case UseCache::No:
      return "no";
      break;
  }
}
std::string Config::emitDebugInfoToString(EmitDebugInfo emitDebugInfo) {
  switch (emitDebugInfo) {
    case EmitDebugInfo::Yes:
      return "yes";
      break;

    case EmitDebugInfo::No:
      return "no";
      break;
  }
}
std::string Config::diagnosticsToString(Diagnostics diagnostics) {
  switch (diagnostics) {
    case Diagnostics::None: {
      return "none";
    }
    case Diagnostics::Survived: {
      return "survived";
    }
    case Diagnostics::Killed: {
      return "killed";
    }
    case Diagnostics::All: {
      return "all";
    }
  }
}
// Constructor initializes defaults.
// TODO: Refactoring into constants.
Config::Config() :
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
  junkDetection()
{}

Config::Config(const std::string &bitcodeFileList,
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
               JunkDetectionConfig junkDetection) :
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
junkDetection(junkDetection)
{
}

const std::string &Config::getBitcodeFileList() const {
  return bitcodeFileList;
}

std::vector<std::string> Config::getBitcodePaths() const {
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

const std::string &Config::getProjectName() const {
  return projectName;
}

const std::string &Config::getTestFramework() const {
  return testFramework;
}

const std::string &Config::getDynamicLibraries() const {
  return dynamicLibraryFileList;
}

std::vector<std::string> Config::getDynamicLibrariesPaths() const {
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

const std::string &Config::getObjectFileList() const {
  return objectFileList;
}

std::vector<std::string> Config::getObjectFilesPaths() const {
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

const std::vector<std::string> &Config::getMutators() const {
  return mutators;
}

const std::vector<std::string> &Config::getReporters() const {
  return reporters;
}

const std::vector<std::string> &Config::getTests() const {
  return tests;
}

const std::vector<std::string> &Config::getExcludeLocations() const {
  return excludeLocations;
}

const std::vector<CustomTestDefinition> &Config::getCustomTests() const {
  return customTests;
}

bool Config::forkEnabled() const {
  return fork == Fork::Enabled;
}

int Config::getTimeout() const {
  return timeout;
}

bool Config::cachingEnabled() const {
  return caching == UseCache::Yes;
}

bool Config::dryRunModeEnabled() const {
  return dryRun == DryRunMode::Enabled;
}

bool Config::failFastModeEnabled() const {
  return failFast == FailFastMode::Enabled;
}

bool Config::shouldEmitDebugInfo() const {
  return emitDebugInfo == EmitDebugInfo::Yes;
}

bool Config::junkDetectionEnabled() const {
  return junkDetection.isEnabled();
}

JunkDetectionConfig &Config::junkDetectionConfig() {
  return junkDetection;
}

Config::Diagnostics Config::getDiagnostics() const {
  return diagnostics;
}

int Config::getMaxDistance() const {
  return maxDistance;
}

std::string Config::getCacheDirectory() const {
  return cacheDirectory;
}

void Config::dump() const {
  Logger::debug() << "Config>\n"
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

  if (mutators.empty() == false) {
    Logger::debug() << "\t" << "mutators: " << '\n';

    for (auto mutator : mutators) {
      Logger::debug() << "\t- " << mutator << '\n';
    }
  }

  if (getTests().empty() == false) {
    Logger::debug() << "\t" << "tests: " << '\n';

    for (auto test : getTests()) {
      Logger::debug() << "\t- " << test << '\n';
    }
  }

  if (getExcludeLocations().empty() == false) {
    Logger::debug() << "\t" << "exclude_locations: " << '\n';

    for (auto excludeLocation : getExcludeLocations()) {
      Logger::debug() << "\t- " << excludeLocation << '\n';
    }
  }
}

std::vector<std::string> Config::validate() {
  std::vector<std::string> errors;

  if (bitcodeFileList.size() == 0) {
    std::string error = "bitcode_file_list parameter is not specified.";
    errors.push_back(error);
    return errors;
  }

  std::ifstream bitcodeFile(bitcodeFileList.c_str());

  if (bitcodeFile.good() == false) {
    std::stringstream error;

    error << "bitcode_file_list parameter points to a non-existing file: "
    << bitcodeFileList;

    errors.push_back(error.str());
  }

  if (dynamicLibraryFileList.empty() == false) {
    std::ifstream dynamicLibraryFile(dynamicLibraryFileList.c_str());

    if (dynamicLibraryFile.good() == false) {
      std::stringstream error;

      error << "dynamic_library_file_list parameter points to a non-existing file: "
      << dynamicLibraryFileList;

      errors.push_back(error.str());
    }
  }

  if (objectFileList.empty() == false) {
    std::ifstream objectFiles(objectFileList.c_str());

    if (objectFiles.good() == false) {
      std::stringstream error;

      error << "object_file_list parameter points to a non-existing file: "
      << objectFileList;

      errors.push_back(error.str());
    }
  }

  return errors;
}

