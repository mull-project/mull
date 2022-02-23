#include "mull/JunkDetection/CXX/CompilationDatabase.h"
#include "mull/Config/Configuration.h"
#include "mull/Diagnostics/Diagnostics.h"
#include "mull/Runner.h"
#include <clang/Tooling/CompilationDatabase.h>
#include <clang/Tooling/JSONCompilationDatabase.h>
#include <llvm/ADT/SmallString.h>
#include <llvm/Support/Path.h>

#include <algorithm>
#include <iterator>
#include <string>

using namespace mull;
using namespace std::string_literals;

static CompilationDatabase::CompilerAndFlags filterFlags(const std::vector<std::string> &flags) {
  if (flags.empty()) {
    return {};
  }
  return { flags.front(), { ++flags.begin(), flags.end() } };
}

static CompilationDatabase::Flags flagsFromString(const std::string &s) {
  std::istringstream iss{ s };
  return CompilationDatabase::Flags(std::istream_iterator<std::string>{ iss }, {});
}

static CompilationDatabase::CompilerAndFlags
flagsFromCommand(const clang::tooling::CompileCommand &command,
                 const CompilationDatabase::Flags &extraFlags) {
  auto [compiler, flags] = filterFlags(command.CommandLine);
  std::copy(std::begin(extraFlags), std::end(extraFlags), std::back_inserter(flags));
  return { compiler, flags };
}

static CompilationDatabase::Database
prepareDatabase(Diagnostics &diagnostics,
                const std::vector<clang::tooling::CompileCommand> &commands,
                const CompilationDatabase::Flags &extraFlags) {
  CompilationDatabase::Database database;
  for (const auto &command : commands) {
    std::string filePath = command.Filename;
    if (!filePath.empty() && !llvm::sys::path::is_absolute(filePath)) {
      filePath = command.Directory + llvm::sys::path::get_separator().str() + filePath;
    }
    database[filePath] = flagsFromCommand(command, extraFlags);
  }
  return database;
}

static CompilationDatabase::Database
createBitcodeFlags(Diagnostics &diagnostics,
                   const std::unordered_map<std::string, std::string> &bitcodeFlagsMap,
                   const CompilationDatabase::Flags &extraFlags) {

  CompilationDatabase::Database mergedBitcodeFlags;
  for (auto const &[filename, commandline] : bitcodeFlagsMap) {
    CompilationDatabase::Flags fileFlags = flagsFromString(commandline);

    auto [compiler, flags] = filterFlags(fileFlags);

    for (const auto &extraFlag : extraFlags) {
      flags.push_back(extraFlag);
    }

    mergedBitcodeFlags[filename] = { compiler, flags };
  }

  return mergedBitcodeFlags;
}

static void resolveResourceDir(Diagnostics &diagnostics, CompilationDatabase::Database &database,
                               std::unordered_map<std::string, std::string> &resourceDirs) {
  for (auto &[filename, compilerAndFlags] : database) {
    auto &[compiler, flags] = compilerAndFlags;
    if (compiler.empty()) {
      continue;
    }
    bool needsResourceDir = true;
    for (auto &flag : flags) {
      if (llvm::StringRef(flag).endswith("-resource-dir")) {
        needsResourceDir = false;
      }
    }
    if (!needsResourceDir) {
      continue;
    }
    if (resourceDirs.count(compiler) == 0) {
      Runner runner(diagnostics);
      auto result = runner.runProgram(compiler,
                                      { "-print-resource-dir" },
                                      {},
                                      mull::MullDefaultTimeoutMilliseconds,
                                      true,
                                      true,
                                      std::nullopt);
      if (!result.stdoutOutput.empty()) {
        // strip trailing \n
        result.stdoutOutput[result.stdoutOutput.size() - 1] = '\0';
      }
      resourceDirs[compiler] = result.stdoutOutput;
    }

    if (!resourceDirs[compiler].empty()) {
      flags.push_back("-resource-dir");
      flags.push_back(resourceDirs[compiler]);
    }
  }
}

static CompilationDatabase
createFromClangCompDB(Diagnostics &diagnostics,
                      std::unique_ptr<clang::tooling::JSONCompilationDatabase> jsondb,
                      const std::string &extraFlags,
                      const std::unordered_map<std::string, std::string> &bitcodeFlags) {
  auto extra = flagsFromString(extraFlags);
  auto bitcodeDatabase = createBitcodeFlags(diagnostics, bitcodeFlags, extra);
  std::vector<clang::tooling::CompileCommand> commands;
  if (jsondb) {
    commands = jsondb->getAllCompileCommands();
  }
  auto database = prepareDatabase(diagnostics, commands, extra);

  std::unordered_map<std::string, std::string> resourceDirs;
  resolveResourceDir(diagnostics, database, resourceDirs);
  resolveResourceDir(diagnostics, bitcodeDatabase, resourceDirs);
  return CompilationDatabase(database, extra, bitcodeDatabase);
}

CompilationDatabase
CompilationDatabase::fromFile(Diagnostics &diagnostics, const std::string &path,
                              const std::string &extraFlags,
                              const std::unordered_map<std::string, std::string> &bitcodeFlags) {
  Database database;
  std::unique_ptr<clang::tooling::JSONCompilationDatabase> jsondb{ nullptr };
  if (!path.empty()) {
    std::string errorMessage;
    jsondb = clang::tooling::JSONCompilationDatabase::loadFromFile(
        path, errorMessage, clang::tooling::JSONCommandLineSyntax::AutoDetect);
    if (!jsondb) {
      diagnostics.warning("Can not read compilation database: "s + errorMessage);
    }
  }
  return createFromClangCompDB(diagnostics, std::move(jsondb), extraFlags, bitcodeFlags);
}

CompilationDatabase
CompilationDatabase::fromBuffer(Diagnostics &diagnostics, const std::string &buffer,
                                const std::string &extraFlags,
                                const std::unordered_map<std::string, std::string> &bitcodeFlags) {
  std::string errorMessage;
  auto jsondb = clang::tooling::JSONCompilationDatabase::loadFromBuffer(
      buffer, errorMessage, clang::tooling::JSONCommandLineSyntax::AutoDetect);
  if (!jsondb) {
    diagnostics.warning("Can not parse compilation database: "s + errorMessage);
  }
  return createFromClangCompDB(diagnostics, std::move(jsondb), extraFlags, bitcodeFlags);
}

CompilationDatabase::CompilationDatabase(CompilationDatabase::Database database,
                                         CompilationDatabase::Flags extraFlags,
                                         CompilationDatabase::Database bitcodeFlags)
    : extraFlags(std::move(extraFlags)), database(std::move(database)),
      bitcodeFlags(std::move(bitcodeFlags)), fallback({ "mull", this->extraFlags }) {}

const CompilationDatabase::CompilerAndFlags &
CompilationDatabase::compilationFlagsForFile(const std::string &filepath) const {
  if (database.empty() && bitcodeFlags.empty()) {
    return fallback;
  }

  /// Look in bitcode flags
  auto it = bitcodeFlags.find(filepath);
  if (it != bitcodeFlags.end()) {
    return it->second;
  }
  auto filename = llvm::sys::path::filename(filepath);
  it = bitcodeFlags.find(filename.str());
  if (it != bitcodeFlags.end()) {
    return it->second;
  }

  llvm::SmallString<128> dotlessPath(filepath);
  llvm::sys::path::remove_dots(dotlessPath, true);
  it = bitcodeFlags.find(dotlessPath.str().str());
  if (it != bitcodeFlags.end()) {
    return it->second;
  }

  /// Look in compilation database
  it = database.find(filepath);
  if (it != database.end()) {
    return it->second;
  }
  filename = llvm::sys::path::filename(filepath);
  it = database.find(filename.str());
  if (it != database.end()) {
    return it->second;
  }

  llvm::sys::path::remove_dots(dotlessPath, true);
  it = database.find(dotlessPath.str().str());
  if (it != database.end()) {
    return it->second;
  }

  return fallback;
}
