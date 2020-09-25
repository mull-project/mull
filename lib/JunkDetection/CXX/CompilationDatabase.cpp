#include "mull/JunkDetection/CXX/CompilationDatabase.h"
#include "CompilationDatabaseParser.h"
#include "mull/Diagnostics/Diagnostics.h"
#include <llvm/ADT/SmallString.h>
#include <llvm/Support/Path.h>

#include <algorithm>
#include <iterator>
#include <regex>
#include <sstream>
#include <string>

using namespace mull;
using namespace std::string_literals;

/// Compilation databases such as one created by CMake can be a subject of the
/// following:
///
///     Parameters use shell quoting and shell escaping of quotes, with ‘"’ and
///     ‘\’ being the only special characters
///     https://clang.llvm.org/docs/JSONCompilationDatabase.html#format
///
/// The compile DB uses shell escaping, and this needs to be undone because
/// clang::ASTUnit::LoadFromCommandLine() doesn't execute commands in shell.
///
/// See "Problem with compiler defines and use of '\"'" (#678) for details.
/// https://github.com/mull-project/mull/issues/678
///
/// Example:
/// {
///   "directory": "/src/builds/amd64-linux-mull",
///   "command":
///     "/usr/bin/clang++-9 -DQT_TESTCASE_BUILDDIR=\\\"/src/builds/amd64-linux-mull\\\" ...",
///   "file": /src/tests/TestingUnitTests/ModuleTestSuite.cpp"
/// }
static std::string undoEscapingString(std::string &string) {
  std::string unescaped(std::regex_replace(string, std::regex(R"(\\\")"), "\""));
  unescaped = std::regex_replace(unescaped, std::regex(R"(\\\\)"), "\\");
  return unescaped;
}

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

static CompilationDatabase::Flags flagsFromString(std::string s) {
  std::istringstream iss{ std::move(s) };
  return CompilationDatabase::Flags(std::istream_iterator<std::string>{ iss }, {});
}

static CompilationDatabase::Flags flagsFromCommand(const CompileCommand &command,
                                                   const CompilationDatabase::Flags &extraFlags) {
  CompilationDatabase::Flags flags(command.arguments);
  if (flags.empty()) {
    flags = flagsFromString(command.command);
  }
  flags = filterFlags(flags, true);

  // The compilation database produced from running
  // clang ... -MJ <comp.database.json>
  // contains a file name in the "arguments" array. Since the file name
  // itself is not a compilation flag we filter it out.
  flags.erase(std::remove(flags.begin(), flags.end(), command.file), flags.end());

  for (auto &flag : flags) {
    flag = undoEscapingString(flag);
  }

  // append extraFlags
  std::copy(std::begin(extraFlags), std::end(extraFlags), std::back_inserter(flags));

  return flags;
}

static CompilationDatabase::Database parseBuffer(Diagnostics &diagnostics, llvm::StringRef buffer,
                                                 const CompilationDatabase::Flags &extraFlags) {
  CompilationDatabase::Database database;
  llvm::yaml::Input json(buffer);

  std::vector<CompileCommand> commands;
  json >> commands;

  if (auto error = json.error()) {
    diagnostics.warning("Can not parse compilation database: "s + error.message());
  }

  for (const auto &command : commands) {
    std::string filePath = command.file;
    if (!filePath.empty() && !llvm::sys::path::is_absolute(filePath)) {
      filePath = command.directory + llvm::sys::path::get_separator().str() + filePath;
    }
    database[filePath] = flagsFromCommand(command, extraFlags);
  }

  return database;
}

static std::map<std::string, std::vector<std::string>>
createBitcodeFlags(Diagnostics &diagnostics, const std::map<std::string, std::string> &bitcodeFlagsMap,
                   const CompilationDatabase::Flags &extraFlags) {

  std::map<std::string, std::vector<std::string>> mergedBitcodeFlags;

  for (auto const &bitcodeFileEntry : bitcodeFlagsMap) {
    CompilationDatabase::Flags fileFlags = flagsFromString(bitcodeFileEntry.second);

    fileFlags = filterFlags(fileFlags, true);

    /// Remove file name from the list of flags
    fileFlags.erase(std::remove(fileFlags.begin(), fileFlags.end(), bitcodeFileEntry.first),
                    fileFlags.end());

    for (const auto &extraFlag : extraFlags) {
      fileFlags.push_back(extraFlag);
    }

    mergedBitcodeFlags[bitcodeFileEntry.first] = fileFlags;
  }

  return mergedBitcodeFlags;
}

static CompilationDatabase
parseDatabaseFromBuffer(Diagnostics &diagnostics, llvm::StringRef buffer, std::string extraFlags,
                        const std::map<std::string, std::string> &bitcodeFlags) {
  CompilationDatabase::Flags _extraFlags(flagsFromString(std::move(extraFlags)));
  CompilationDatabase::Database database(parseBuffer(diagnostics, buffer, _extraFlags));
  CompilationDatabase::Database _bitcodeFlags(
      createBitcodeFlags(diagnostics, bitcodeFlags, _extraFlags));
  return CompilationDatabase(std::move(database), std::move(_extraFlags), std::move(_bitcodeFlags));
}

CompilationDatabase
CompilationDatabase::fromFile(Diagnostics &diagnostics, const std::string &path,
                              std::string extraFlags,
                              const std::map<std::string, std::string> &bitcodeFlags) {
  if (!path.empty()) {
    auto bufferOrError = llvm::MemoryBuffer::getFile(path);
    if (bufferOrError) {
      auto buffer = std::move(*bufferOrError);
      return parseDatabaseFromBuffer(
          diagnostics, buffer->getBuffer(), std::move(extraFlags), bitcodeFlags);
    }
    diagnostics.warning("Can not read compilation database "s + path + ": " +
                        bufferOrError.getError().message());
  }
  return parseDatabaseFromBuffer(diagnostics, "", std::move(extraFlags), bitcodeFlags);
}

CompilationDatabase
CompilationDatabase::fromBuffer(Diagnostics &diagnostics, const std::string &buffer,
                                std::string extraFlags,
                                const std::map<std::string, std::string> &bitcodeFlags) {
  return parseDatabaseFromBuffer(diagnostics, buffer, std::move(extraFlags), bitcodeFlags);
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
  it = bitcodeFlags.find(filename);
  if (it != bitcodeFlags.end()) {
    return it->second;
  }

  llvm::SmallString<128> dotlessPath(filepath);
  llvm::sys::path::remove_dots(dotlessPath, true);
  it = bitcodeFlags.find(dotlessPath.str());
  if (it != bitcodeFlags.end()) {
    return it->second;
  }

  /// Look in compilation database
  it = database.find(filepath);
  if (it != database.end()) {
    return it->second;
  }
  filename = llvm::sys::path::filename(filepath);
  it = database.find(filename);
  if (it != database.end()) {
    return it->second;
  }

  llvm::sys::path::remove_dots(dotlessPath, true);
  it = database.find(dotlessPath.str());
  if (it != database.end()) {
    return it->second;
  }

  return extraFlags;
}
