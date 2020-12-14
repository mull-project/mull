#include "mull/JunkDetection/CXX/CompilationDatabase.h"
#include "mull/Diagnostics/Diagnostics.h"
#include <clang/Tooling/CompilationDatabase.h>
#include <clang/Tooling/JSONCompilationDatabase.h>
#include <llvm/ADT/SmallString.h>
#include <llvm/Support/Path.h>

#include <algorithm>
#include <iterator>
#include <sstream>
#include <string>

using namespace mull;
using namespace std::string_literals;

static std::vector<std::string> filterFlags(const std::vector<std::string> &flags,
                                            bool stripCompiler) {
  if (flags.empty()) {
    return flags;
  }
  auto it = flags.begin();
  auto end = flags.end();

  if (stripCompiler) {
    // Skip past the compiler command
    ++it;
  }

  return CompilationDatabase::Flags(it, end);
}

static CompilationDatabase::Flags flagsFromString(const std::string &s) {
  std::istringstream iss{ s };
  return CompilationDatabase::Flags(std::istream_iterator<std::string>{ iss }, {});
}

static CompilationDatabase::Flags flagsFromCommand(const clang::tooling::CompileCommand &command,
                                                   const CompilationDatabase::Flags &extraFlags) {
  CompilationDatabase::Flags flags(command.CommandLine);
  flags = filterFlags(flags, true);

  // The compilation database produced from running
  // clang ... -MJ <comp.database.json>
  // contains a file name in the "arguments" array. Since the file name
  // itself is not a compilation flag we filter it out.
  flags.erase(std::remove(flags.begin(), flags.end(), command.Filename), flags.end());

  // append extraFlags
  std::copy(std::begin(extraFlags), std::end(extraFlags), std::back_inserter(flags));

  return flags;
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

static std::map<std::string, std::vector<std::string>>
createBitcodeFlags(Diagnostics &diagnostics, const std::map<std::string, std::string> &bitcodeFlagsMap,
                   const CompilationDatabase::Flags &extraFlags) {

  std::map<std::string, std::vector<std::string>> mergedBitcodeFlags;

  for (auto const &[filename, commandline] : bitcodeFlagsMap) {
    CompilationDatabase::Flags fileFlags = flagsFromString(commandline);

    fileFlags = filterFlags(fileFlags, true);

    /// Remove file name from the list of flags
    fileFlags.erase(std::remove(fileFlags.begin(), fileFlags.end(), filename),
                    fileFlags.end());

    for (const auto &extraFlag : extraFlags) {
      fileFlags.push_back(extraFlag);
    }

    mergedBitcodeFlags[filename] = fileFlags;
  }

  return mergedBitcodeFlags;
}

static CompilationDatabase createFromClangCompDB(
    Diagnostics &diagnostics, std::unique_ptr<clang::tooling::JSONCompilationDatabase> jsondb,
    const std::string &extraFlags, const std::map<std::string, std::string> &bitcodeFlags) {
  auto _extraFlags = flagsFromString(extraFlags);
  auto _bitcodeFlags = createBitcodeFlags(diagnostics, bitcodeFlags, _extraFlags);
  std::vector<clang::tooling::CompileCommand> commands;
  if (jsondb) {
    commands = jsondb->getAllCompileCommands();
  }
  auto database = prepareDatabase(diagnostics, commands, _extraFlags);
  return CompilationDatabase(database, _extraFlags, _bitcodeFlags);
}

CompilationDatabase
CompilationDatabase::fromFile(Diagnostics &diagnostics, const std::string &path,
                              const std::string &extraFlags,
                              const std::map<std::string, std::string> &bitcodeFlags) {
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
                                const std::map<std::string, std::string> &bitcodeFlags) {
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
      bitcodeFlags(std::move(bitcodeFlags)) {}

const CompilationDatabase::Flags &
CompilationDatabase::compilationFlagsForFile(const std::string &filepath) const {
  if (database.empty() && bitcodeFlags.empty()) {
    return extraFlags;
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

  return extraFlags;
}
