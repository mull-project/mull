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

void addHeaderSearchPathsFromCompiler(std::vector<std::string> &flags,
                                      const std::string &compiler) {
  if (compiler.empty()) {
    return;
  }

  auto binDirectory = llvm::sys::path::parent_path(compiler);
  if (binDirectory.empty()) {
    return;
  }

  auto installDirectory = llvm::sys::path::parent_path(binDirectory);
  llvm::SmallString<128> cppIncludeDir;
  llvm::sys::path::append(cppIncludeDir, installDirectory, "include", "c++",
                          "v1");

  llvm::SmallString<128> cIncludeDir;
  llvm::sys::path::append(cIncludeDir, installDirectory, "lib", "clang",
                          CLANG_VERSION_STRING);
  llvm::sys::path::append(cIncludeDir, "include");

  flags.emplace_back("-I");
  flags.emplace_back(cppIncludeDir.c_str());
  flags.emplace_back("-I");
  flags.emplace_back(cIncludeDir.c_str());
}

static std::vector<std::string>
flagsFromCommand(const CompileCommand &command) {
  const std::string &input = commandOrArguments(command);

  std::istringstream stream(input);
  std::string compiler;
  stream >> compiler;

  std::string rest;
  std::getline(stream, rest);

  std::vector<std::string> flags = flagsFromString(rest);
  addHeaderSearchPathsFromCompiler(flags, compiler);
  return flags;
}

static std::map<std::string, std::vector<std::string>>
loadDatabaseFromFile(const std::string &path) {
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
    database[command.file] = flagsFromCommand(command);
  }

  return database;
}

CompilationDatabase::CompilationDatabase(CompilationDatabase::Path path,
                                         CompilationDatabase::Flags flags)
    : flags(flagsFromString(flags.getFlags())),
      database(loadDatabaseFromFile(path.getPath())) {}

const std::vector<std::string> &CompilationDatabase::compilationFlagsForFile(
    const std::string &filepath) const {
  if (database.empty()) {
    return flags;
  }

  auto it = database.find(filepath);
  if (it == database.end()) {
    return flags;
  }

  return it->second;
}
