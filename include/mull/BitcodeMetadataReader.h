#pragma once

#include <map>
#include <memory>
#include <vector>

namespace mull {
class Bitcode;

class BitcodeMetadataReader {
public:
  std::map<std::string, std::string>
  getCompilationDatabase(std::vector<std::unique_ptr<mull::Bitcode>> &bitcode);
};

} // namespace mull
