#include "mull/Filters/FilePathFilter.h"
#include "mull/MutationPoint.h"
#include "mull/SourceLocation.h"

#include <cassert>
#include <llvm/Support/raw_ostream.h>

using namespace mull;

bool FilePathFilter::shouldSkip(MutationPoint *point) {
  return shouldSkip(point->getSourceLocation());
}

bool FilePathFilter::shouldSkip(llvm::Function *function) {
  SourceLocation location = SourceLocation::locationFromFunction(function);
  return shouldSkip(location);
}

bool FilePathFilter::shouldSkip(llvm::Instruction *instruction) const {
  SourceLocation location =
      SourceLocation::locationFromInstruction(instruction);
  return shouldSkip(location);
}

bool FilePathFilter::shouldSkip(const mull::SourceLocation &location) const {
  for (const auto &r : filters) {
    assert(!location.isNull());
    auto path = location.filePath;
    if (std::regex_search(path, r)) {
      return true;
    }
  }
  return false;
}

std::string FilePathFilter::name() { return "file path"; }

void FilePathFilter::exclude(const std::string &filter) {
  filters.push_back(std::regex(filter, std::regex::grep));
}
