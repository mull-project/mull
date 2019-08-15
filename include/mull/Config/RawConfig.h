#pragma once

#include "mull/Config/ConfigurationOptions.h"
#include "mull/Logger.h"

#include <cstddef>
#include <fstream>
#include <llvm/Support/YAMLTraits.h>
#include <sstream>
#include <string>
#include <vector>

extern int MullDefaultTimeoutMilliseconds;

// We need these forward declarations to make our config friends with the
// mapping traits.
namespace mull {
class RawConfig;
}
namespace llvm {
namespace yaml {
template <typename T> struct MappingTraits;
}
} // namespace llvm
namespace mull {

struct JunkDetectionConfig {
  enum class JunkDetectionToggle { Enabled, Disabled };

  JunkDetectionToggle toggle;
  std::string detectorName;
  std::string cxxCompilationDatabasePath;
  std::string cxxCompilationFlags;

  JunkDetectionConfig();
  bool isEnabled() const;

  static JunkDetectionConfig enabled();
  static JunkDetectionConfig disabled();
};

class RawConfig {
public:
  enum class Fork { Disabled, Enabled };
  enum class DryRunMode { Disabled, Enabled };
  enum class FailFastMode { Disabled, Enabled };
  enum class UseCache { No, Yes };
  enum class EmitDebugInfo { No, Yes };

  static std::string forkToString(Fork fork);
  static std::string dryRunToString(DryRunMode dryRun);
  static std::string failFastToString(FailFastMode failFast);
  static std::string cachingToString(UseCache caching);
  static std::string emitDebugInfoToString(EmitDebugInfo emitDebugInfo);

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
  ParallelizationConfig parallelizationConfig;

  std::string mutationTestingElementsReportPath;

  friend llvm::yaml::MappingTraits<mull::RawConfig>;

public:
  RawConfig();

  RawConfig(const std::string &bitcodeFileList, const std::string &project,
            const std::string &testFramework,
            const std::vector<std::string> mutators,
            const std::vector<std::string> reporters,
            const std::string &dynamicLibraryFileList,
            const std::string &objectFileList,
            const std::vector<std::string> tests,
            const std::vector<std::string> excludeLocations,
            const std::vector<CustomTestDefinition> definitions, Fork fork,
            DryRunMode dryRun, FailFastMode failFast, UseCache cache,
            EmitDebugInfo debugInfo, Diagnostics diagnostics, int timeout,
            int distance, const std::string &cacheDir,
            JunkDetectionConfig junkDetection,
            ParallelizationConfig parallelizationConfig,
            const std::string &mutationTestingElementsReportPath);

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

  void addCustomTest(CustomTestDefinition customTest);

  JunkDetectionConfig &junkDetectionConfig();
  Diagnostics getDiagnostics() const;
  const ParallelizationConfig parallelization() const;

  int getTimeout() const;
  int getMaxDistance() const;

  bool forkEnabled() const;
  bool cachingEnabled() const;
  bool dryRunModeEnabled() const;
  bool failFastModeEnabled() const;
  bool shouldEmitDebugInfo() const;
  bool junkDetectionEnabled() const;

  const std::string &getMutationTestingElementsReportPath() const;

  void normalizeParallelizationConfig();

  std::vector<std::string> validate();
  void dump() const;
};
} // namespace mull
