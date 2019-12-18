#include "mull/JunkDetection/CXX/CompilationDatabase.h"
#include "CompilationDatabaseParser.h"
#include "mull/Diagnostics/Diagnostics.h"
#include <llvm/ADT/SmallString.h>
#include <llvm/Support/Path.h>

#include <sstream>

using namespace mull;

static std::vector<std::string> filterFlags(const std::vector<std::string> &flags,
                                            bool stripCompiler) {
  std::vector<std::string> filteredFlags;
  if (flags.empty()) {
    return filteredFlags;
  }
  auto it = flags.begin();
  auto end = flags.end();

  if (stripCompiler) {
    ++it;
  }

  while (it != end) {
    std::string flag = *(it++);
    if (flag == "-c") {
      if (it != end) {
        ++it;
      }
      continue;
    }
    filteredFlags.push_back(flag);
  }
  return filteredFlags;
}

static std::vector<std::string> flagsFromString(const std::string &s) {
  std::vector<std::string> flags;

  std::istringstream stream(s);
  while (!stream.eof()) {
    std::string value;
    stream >> value;
    if (value.empty()) {
      continue;
    }
    flags.push_back(value);
  }

  return flags;
}

static std::vector<std::string> flagsFromCommand(const CompileCommand &command) {
  if (command.arguments.empty()) {
    return filterFlags(flagsFromString(command.command), true);
  }
  return filterFlags(command.arguments, true);
}

static std::vector<std::string> flagsFromCommand(const CompileCommand &command,
                                                 const std::vector<std::string> &extraFlags) {
  std::vector<std::string> flags = flagsFromCommand(command);
  /// The compilation database produced from running
  /// clang ... -MJ <comp.database.json>
  /// contains a file name in the "arguments" array. Since the file name
  /// itself is not a compilation flag we filter it out.
  flags.erase(std::remove(flags.begin(), flags.end(), command.file), flags.end());

  for (auto &extra : extraFlags) {
    flags.push_back(extra);
  }
  return flags;
}

static std::map<std::string, std::vector<std::string>>
loadDatabaseFromFile(Diagnostics &diagnostics, const std::string &path,
                     const std::vector<std::string> &extraFlags) {
  std::map<std::string, std::vector<std::string>> database;
  if (path.empty()) {
    return database;
  }

  std::vector<CompileCommand> commands;

  auto bufferOrError = llvm::MemoryBuffer::getFile(path);
  if (!bufferOrError) {
    diagnostics.warning(std::string("Can not read compilation database: ") + path);
  } else {
    auto buffer = bufferOrError->get();
    llvm::yaml::Input json(buffer->getBuffer());

    json >> commands;

    if (json.error()) {
      diagnostics.warning(std::string("Can not read compilation database: ") + path);
    }
  }

  for (auto &command : commands) {
    std::string filePath = command.file;
    if (!filePath.empty() && !llvm::sys::path::is_absolute(filePath)) {
      filePath = command.directory + llvm::sys::path::get_separator().str() + filePath;
    }
    database[filePath] = flagsFromCommand(command, extraFlags);
  }

  return database;
}

CompilationDatabase::CompilationDatabase(Diagnostics &diagnostics, CompilationDatabase::Path path,
                                         CompilationDatabase::Flags flags)
    : extraFlags(filterFlags(flagsFromString(flags.getFlags()), false)),
      database(loadDatabaseFromFile(diagnostics, path.getPath(), extraFlags)) {}

const std::vector<std::string> &
CompilationDatabase::compilationFlagsForFile(const std::string &filepath) const {
  if (database.empty()) {
    return extraFlags;
  }

  auto it = database.find(filepath);
  if (it != database.end()) {
    return it->second;
  }
  auto filename = llvm::sys::path::filename(filepath);
  it = database.find(filename);
  if (it != database.end()) {
    return it->second;
  }

  llvm::SmallString<128> dotlessPath(filepath);
  llvm::sys::path::remove_dots(dotlessPath, true);
  it = database.find(dotlessPath.str());
  if (it != database.end()) {
    return it->second;
  }

  return extraFlags;
}
