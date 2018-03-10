#pragma once

#include "Logger.h"

#include <llvm/Support/YAMLTraits.h>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

extern int MullDefaultTimeoutMilliseconds;

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

  CustomTestDefinition();
  CustomTestDefinition(const std::string &name,
                       const std::string &method,
                       const std::string &program,
                       const std::vector<std::string> arguments);
};

struct JunkDetectionConfig {
  enum class JunkDetectionToggle {
    Enabled,
    Disabled
  };

  JunkDetectionToggle toggle;
  std::string detectorName;
  std::string cxxCompDBDirectory;
  std::string cxxCompilationFlags;

  JunkDetectionConfig();
  bool isEnabled() const;

  static JunkDetectionConfig enabled();
  static JunkDetectionConfig disabled();
};

class Config {
public:
  enum class Fork {
    Disabled,
    Enabled
  };
  enum class DryRunMode {
    Disabled,
    Enabled
  };
  enum class FailFastMode {
    Disabled,
    Enabled
  };
  enum class UseCache {
    No,
    Yes
  };
  enum class EmitDebugInfo {
    No,
    Yes
  };
  enum class Diagnostics {
    None,
    Survived,
    Killed,
    All
  };

  static std::string forkToString(Fork fork);
  static std::string dryRunToString(DryRunMode dryRun);
  static std::string failFastToString(FailFastMode failFast);
  static std::string cachingToString(UseCache caching);
  static std::string emitDebugInfoToString(EmitDebugInfo emitDebugInfo);
  static std::string diagnosticsToString(Diagnostics diagnostics);
private:
  std::string bitcodeFileList;

  std::string projectName;
  std::string testFramework;

  std::vector<std::string> mutators;
  std::vector<std::string> reporters;
  std::string dynamicLibraryFileList;
  std::string objectFileList;
  std::vector<std::string> tests;
  std::vector<std::string> excludeLocations;
  std::vector<CustomTestDefinition> customTests;

  Fork fork;
  DryRunMode dryRun;
  FailFastMode failFast;
  UseCache caching;
  EmitDebugInfo emitDebugInfo;
  Diagnostics diagnostics;

  int timeout;
  int maxDistance;
  std::string cacheDirectory;

  JunkDetectionConfig junkDetection;

  friend llvm::yaml::MappingTraits<mull::Config>;
public:
  Config();

  Config(const std::string &bitcodeFileList,
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
         JunkDetectionConfig junkDetection);

  const std::string &getProjectName() const;
  const std::string &getTestFramework() const;
  std::string getCacheDirectory() const;

  const std::string &getBitcodeFileList() const;
  const std::string &getObjectFileList() const;
  const std::string &getDynamicLibraries() const;

  std::vector<std::string> getBitcodePaths() const;
  std::vector<std::string> getObjectFilesPaths() const;
  std::vector<std::string> getDynamicLibrariesPaths() const;
  
  const std::vector<std::string> &getMutators() const;
  const std::vector<std::string> &getReporters() const;
  const std::vector<std::string> &getTests() const;
  const std::vector<std::string> &getExcludeLocations() const;

  const std::vector<CustomTestDefinition> &getCustomTests() const;

  JunkDetectionConfig &junkDetectionConfig();
  Diagnostics getDiagnostics() const;

  int getTimeout() const;
  int getMaxDistance() const;

  bool forkEnabled() const;
  bool cachingEnabled() const;
  bool dryRunModeEnabled() const;
  bool failFastModeEnabled() const;
  bool shouldEmitDebugInfo() const;
  bool junkDetectionEnabled() const;

  std::vector<std::string> validate();
  void dump() const;
};
}
