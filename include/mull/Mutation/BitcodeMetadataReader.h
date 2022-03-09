#pragma once

#include <memory>
#include <unordered_map>
#include <vector>

namespace llvm {
class Module;
}

namespace mull {
class Bitcode;

class BitcodeMetadataReader {
public:
  std::unordered_map<std::string, std::string>
  getCompilationDatabase(std::vector<std::unique_ptr<mull::Bitcode>> &bitcode);
  std::unordered_map<std::string, std::string> getCompilationDatabase(llvm::Module &module);
};

} // namespace mull
