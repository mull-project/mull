#pragma once

#include "Logger.h"

#include <llvm/Support/YAMLTraits.h>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

static int MullDefaultTimeoutMilliseconds = 3000;

// We need these forward declarations to make our config friends with the
// mapping traits.
namespace mull {
class Config;
}
namespace llvm {
namespace yaml {
template<typename T>
struct MappingTraits;
}
}
namespace mull {

struct CustomTestDefinition {
  std::string testName;
  std::string methodName;
  std::string programName;
  std::vector<std::string> callArguments;
  CustomTestDefinition(const std::string &name,
                       const std::string &method,
                       const std::string &program,
                       const std::vector<std::string> arguments)
  : testName(name),
  methodName(method),
  programName(program),
  callArguments(arguments) {}

  CustomTestDefinition() {}
};

class Config {
  std::string bitcodeFileList;

  std::string projectName;
  std::string testFramework;

  std::vector<std::string> mutationOperators;
  std::string dynamicLibraryFileList;
  std::string objectFileList;
  std::vector<std::string> tests;
  std::vector<std::string> excludeLocations;
  std::vector<CustomTestDefinition> customTests;

  bool fork;
  bool dryRun;
  bool useCache;
  bool emitDebugInfo;
  bool diagnostics;

  int timeout;
  int maxDistance;
  std::string cacheDirectory;

  friend llvm::yaml::MappingTraits<mull::Config>;
public:
  // Constructor initializes defaults.
  // TODO: Refactoring into constants.
  Config() :
    bitcodeFileList(""),
    projectName(""),
    testFramework("GoogleTest"),
    mutationOperators(
      // Yaml::Traits stops reading mutation_operators from config.yaml
      // if these 3 default operators are set here (BUG?).
      // So leaving out the empty ()
      // {
      //   MathAddMutationOperator::ID,
      //   NegateConditionMutationOperator::ID,
      //   RemoveVoidFunctionMutationOperator::ID
      // }
    ),
    dynamicLibraryFileList(),
    objectFileList(),
    tests(),
    excludeLocations(),
    customTests(),
    fork(true),
    dryRun(false),
    useCache(false),
    emitDebugInfo(false),
    diagnostics(false),
    timeout(MullDefaultTimeoutMilliseconds),
    maxDistance(128),
    cacheDirectory("/tmp/mull_cache")
  {
  }

  Config(const std::string &bitcodeFileList,
         const std::string &project,
         const std::string &testFramework,
         const std::vector<std::string> mutationOperators,
         const std::string &dynamicLibraryFileList,
         const std::string &objectFileList,
         const std::vector<std::string> tests,
         const std::vector<std::string> excludeLocations,
         const std::vector<CustomTestDefinition> definitions,
         bool fork,
         bool dryrun,
         bool cache,
         bool debugInfo,
         bool diagnostics,
         int timeout,
         int distance,
         const std::string &cacheDir) :
    bitcodeFileList(bitcodeFileList),
    projectName(project),
    testFramework(testFramework),
    mutationOperators(mutationOperators),
    dynamicLibraryFileList(dynamicLibraryFileList),
    objectFileList(objectFileList),
    tests(tests),
    excludeLocations(excludeLocations),
    customTests(definitions),
    fork(fork),
    dryRun(dryrun),
    useCache(cache),
    emitDebugInfo(debugInfo),
    diagnostics(diagnostics),
    timeout(timeout),
    maxDistance(distance),
    cacheDirectory(cacheDir)
  {
  }

  const std::string &getBitcodeFileList() const {
    return bitcodeFileList;
  }

  std::vector<std::string> getBitcodePaths() const {
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

  const std::string &getProjectName() const {
    return projectName;
  }

  const std::string &getTestFramework() const {
    return testFramework;
  }
  
  const std::string &getDynamicLibraries() const {
    return dynamicLibraryFileList;
  }
  
  std::vector<std::string> getDynamicLibrariesPaths() const {
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

  const std::string &getObjectFileList() const {
    return objectFileList;
  }

  std::vector<std::string> getObjectFilesPaths() const {
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
  
  const std::vector<std::string> &getMutationOperators() const {
    return mutationOperators;
  }

  const std::vector<std::string> &getTests() const {
    return tests;
  }

  const std::vector<std::string> &getExcludeLocations() const {
    return excludeLocations;
  }

  const std::vector<CustomTestDefinition> &getCustomTests() const {
    return customTests;
  }

  bool getFork() const {
    return fork;
  }

  int getTimeout() const {
    return timeout;
  }

  bool getUseCache() const {
    return useCache;
  }

  bool isDryRun() const {
    return dryRun;
  }

  bool shouldEmitDebugInfo() const {
    return emitDebugInfo;
  }

  bool isDiagnosticsEnabled() const {
    return diagnostics;
  }

  int getMaxDistance() const {
    return maxDistance;
  }

  std::string getCacheDirectory() const {
    return cacheDirectory;
  }

  void dump() const {
    Logger::debug() << "Config>\n"
    << "\t" << "bitcode_file_list: " << bitcodeFileList << '\n'
    << "\t" << "dynamic_library_file_list: " << dynamicLibraryFileList << '\n'
    << "\t" << "object_file_list: " << objectFileList << '\n'
    << "\t" << "project_name: " << getProjectName() << '\n'
    << "\t" << "test_framework: " << getTestFramework() << '\n'
    << "\t" << "distance: " << getMaxDistance() << '\n'
    << "\t" << "dry_run: " << isDryRun() << '\n'
    << "\t" << "fork: " << getFork() << '\n'
    << "\t" << "emit_debug_info: " << shouldEmitDebugInfo() << '\n';

    if (mutationOperators.empty() == false) {
      Logger::debug() << "\t" << "mutation_operators: " << '\n';

      for (auto mutationOperator : mutationOperators) {
        Logger::debug() << "\t- " << mutationOperator << '\n';
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

  std::vector<std::string> validate() {
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

};
}
