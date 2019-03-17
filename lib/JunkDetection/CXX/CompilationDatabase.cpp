#include "JunkDetection/CXX/CompilationDatabase.h"
#include "CompilationDatabaseParser.h"
#include "Logger.h"

#include <clang/Basic/Version.h>
#include <llvm/ADT/STLExtras.h>
#include <llvm/ADT/SmallString.h>
#include <llvm/Support/Path.h>

#include <sstream>

using namespace mull;

/// Skips the next word even if it is several spaces away:
///
/// Examples:
///     ' aaa bbb' ->  ' aaa bbb'
///      ^                   ^
///
///     '   aaa bbb' ->  '   aaa bbb'
///      ^                       ^
static void skipNextWord(std::istringstream &stream) {
  int c = stream.get();
  while (c == ' ') {
    stream.ignore(std::numeric_limits<std::streamsize>::max(), ' ');
    c = stream.get();
  }
  if (stream.good()) {
    stream.unget();
  }
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
    if (value == "-c") {
      skipNextWord(stream);
      continue;
    }
    flags.push_back(value);
  }

  return flags;
}

const std::string &commandOrArguments(const CompileCommand &command) {
  if (command.arguments.empty()) {
    return command.command;
  }
  return command.arguments;
}

static std::vector<std::string>
flagsFromCommand(const CompileCommand &command,
                 const std::vector<std::string> &extraFlags) {
  const std::string &input = commandOrArguments(command);

  std::istringstream stream(input);
  std::string compiler;
  stream >> compiler;

  std::string rest;
  std::getline(stream, rest);

  std::vector<std::string> flags = flagsFromString(rest);
  for (auto &extra : extraFlags) {
    flags.push_back(extra);
  }
  return flags;
}

static std::map<std::string, std::vector<std::string>>
loadDatabaseFromFile(const std::string &path,
                     const std::vector<std::string> &extraFlags) {
  std::map<std::string, std::vector<std::string>> database;
  if (path.empty()) {
    return database;
  }

  std::vector<CompileCommand> commands;

  auto bufferOrError = llvm::MemoryBuffer::getFile(path);
  if (!bufferOrError) {
    Logger::error() << "Can not read compilation database: " << path << '\n';
  } else {
    auto buffer = bufferOrError->get();
    llvm::yaml::Input json(buffer->getBuffer());

    json >> commands;

    if (json.error()) {
      Logger::error() << "Can not read compilation database: " << path << '\n';
    }
  }

  for (auto &command : commands) {
    database[command.file] = flagsFromCommand(command, extraFlags);
  }

  return database;
}

CompilationDatabase::CompilationDatabase(CompilationDatabase::Path path,
                                         CompilationDatabase::Flags flags)
    : extraFlags(flagsFromString(flags.getFlags())),
      database(loadDatabaseFromFile(path.getPath(), extraFlags)) {}

const std::vector<std::string> &CompilationDatabase::compilationFlagsForFile(
    const std::string &filepath) const {
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

  return extraFlags;
}
