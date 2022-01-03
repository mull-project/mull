#include "mull/Config/Configuration.h"
#include "mull/Diagnostics/Diagnostics.h"

#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Path.h>
#include <llvm/Support/YAMLTraits.h>

using namespace mull;
using namespace std::string_literals;

template <> struct llvm::yaml::ScalarEnumerationTraits<IDEDiagnosticsKind> {
  static void enumeration(llvm::yaml::IO &io, IDEDiagnosticsKind &value) {
    io.enumCase(value, "none", IDEDiagnosticsKind::None);
    io.enumCase(value, "killed", IDEDiagnosticsKind::Killed);
    io.enumCase(value, "survived", IDEDiagnosticsKind::Survived);
    io.enumCase(value, "all", IDEDiagnosticsKind::All);
  }
};

template <> struct llvm::yaml::MappingTraits<ParallelizationConfig> {
  static void mapping(llvm::yaml::IO &io, ParallelizationConfig &config) {
    io.mapOptional("workers", config.workers);
    io.mapOptional("testExecutionWorkers", config.testExecutionWorkers);
    io.mapOptional("mutantExecutionWorkers", config.mutantExecutionWorkers);
  }
};

template <> struct llvm::yaml::MappingTraits<Configuration> {
  static void mapping(llvm::yaml::IO &io, Configuration &config) {
    io.mapOptional("debugEnabled", config.debugEnabled);
    io.mapOptional("dryRunEnabled", config.dryRunEnabled);
    io.mapOptional("captureTestOutput", config.captureTestOutput);
    io.mapOptional("captureMutantOutput", config.captureMutantOutput);
    io.mapOptional("skipSanityCheckRun", config.skipSanityCheckRun);
    io.mapOptional("includeNotCovered", config.includeNotCovered);
    io.mapOptional("keepObjectFiles", config.keepObjectFiles);
    io.mapOptional("keepExecutable", config.keepExecutable);
    io.mapOptional("mutateOnly", config.mutateOnly);
    io.mapOptional("lowerBitcode", config.lowerBitcode);
    io.mapOptional("timeout", config.timeout);
    io.mapOptional("linkerTimeout", config.linkerTimeout);
    io.mapOptional("diagnostics", config.diagnostics);
    io.mapOptional("mutators", config.mutators);
    io.mapOptional("executable", config.executable);
    io.mapOptional("outputFile", config.outputFile);
    io.mapOptional("coverageInfo", config.coverageInfo);
    io.mapOptional("linker", config.linker);
    io.mapOptional("linkerFlags", config.linkerFlags);
    io.mapOptional("parallelization", config.parallelization);
    io.mapOptional("compilationDatabasePath", config.compilationDatabasePath);
    io.mapOptional("compilerFlags", config.compilerFlags);
    io.mapOptional("junkDetectionDisabled", config.junkDetectionDisabled);
    io.mapOptional("gitDiffRef", config.gitDiffRef);
    io.mapOptional("gitProjectRoot", config.gitProjectRoot);
    io.mapOptional("includePaths", config.includePaths);
    io.mapOptional("excludePaths", config.excludePaths);
  }
};

std::string Configuration::findConfig(Diagnostics &diagnostics) {
  if (getenv("MULL_CONFIG") != nullptr) {
    return getenv("MULL_CONFIG");
  }
  llvm::SmallString<PATH_MAX> cwd;
  std::error_code ec = llvm::sys::fs::current_path(cwd);
  if (ec) {
    diagnostics.warning("Cannot find config: "s + ec.message());
    return {};
  }
  std::string currentDir = cwd.str().str();
  auto config = currentDir + "/mull.yml";
  if (llvm::sys::fs::exists(config)) {
    return config;
  }
  while (llvm::sys::path::has_parent_path(currentDir)) {
    currentDir = llvm::sys::path::parent_path(currentDir);
    config = currentDir + "/mull.yml";
    if (llvm::sys::fs::exists(config)) {
      return config;
    }
  }
  return {};
}

Configuration Configuration::loadFromDisk(Diagnostics &diagnostics, const std::string &path) {
  auto bufferOrError = llvm::MemoryBuffer::getFile(path);
  if (!bufferOrError) {
    diagnostics.warning("Cannot read config: "s + bufferOrError.getError().message());
    return {};
  }
  llvm::yaml::Input input(bufferOrError.get()->getMemBufferRef());
  Configuration configuration;
  input >> configuration;
  return configuration;
}
