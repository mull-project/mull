#include "mull/MutationFilters/FilePathFilter.h"

#include "mull/MutationPoint.h"

#include <cassert>

using namespace mull;

bool FilePathFilter::shouldSkip(MutationPoint *point) {
  for (auto r : filters) {
    assert(!point->getSourceLocation().isNull());
    auto path = point->getSourceLocation().filePath;
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
