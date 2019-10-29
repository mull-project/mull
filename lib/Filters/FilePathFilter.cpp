#include "mull/Filters/FilePathFilter.h"

#include "mull/MutationPoint.h"

#include <cassert>
#include <llvm/Support/raw_ostream.h>

using namespace mull;

bool FilePathFilter::shouldSkip(MutationPoint *point) {
  for (const auto &r : filters) {
    assert(!point->getSourceLocation().isNull());
    auto path = point->getSourceLocation().filePath;
    if (std::regex_search(path, r)) {
      return true;
    }
  }

  return false;
}

bool FilePathFilter::shouldSkip(llvm::Function *function) {
  for (const auto &r : filters) {
    SourceLocation location = SourceLocation::locationFromFunction(function);
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
