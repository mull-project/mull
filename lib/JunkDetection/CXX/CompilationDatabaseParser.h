#pragma once

#include <llvm/Support/YAMLParser.h>
#include <llvm/Support/YAMLTraits.h>

#include <string>
#include <vector>

namespace mull {

struct CompileCommand {
  std::string directory;
  std::string file;
  std::string output;
  std::string arguments;
  std::string command;
};

} // namespace mull

namespace llvm {
namespace yaml {
class Input;

template <> struct SequenceTraits<std::vector<mull::CompileCommand>> {
  static size_t size(IO &io, std::vector<mull::CompileCommand> &list) {
    return list.size();
  }

  static mull::CompileCommand &
  element(IO &io, std::vector<mull::CompileCommand> &list, size_t index) {
    mull::CompileCommand t;
    list.push_back(t);
    return list.back();
  }
};

template <> struct MappingTraits<mull::CompileCommand> {
  static void mapping(IO &io, mull::CompileCommand &command) {
    io.mapRequired("directory", command.directory);
    io.mapRequired("file", command.file);
    io.mapOptional("output", command.output);
    io.mapOptional("arguments", command.arguments);
    io.mapOptional("command", command.command);
  }
};
} // namespace yaml
} // namespace llvm