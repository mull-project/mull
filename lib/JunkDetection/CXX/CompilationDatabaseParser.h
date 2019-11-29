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
  std::string command;
  std::vector<std::string> arguments;
};

} // namespace mull

#if LLVM_VERSION_MAJOR < 5
LLVM_YAML_IS_SEQUENCE_VECTOR(std::string)
#endif
LLVM_YAML_IS_SEQUENCE_VECTOR(mull::CompileCommand)

namespace llvm {
namespace yaml {
class Input;

template <> struct MappingTraits<mull::CompileCommand> {
  static void mapping(IO &io, mull::CompileCommand &command) {
    io.mapRequired("directory", command.directory);
    io.mapRequired("file", command.file);
    io.mapOptional("output", command.output);
    io.mapOptional("command", command.command);
    io.mapOptional("arguments", command.arguments);
  }
};
} // namespace yaml
} // namespace llvm