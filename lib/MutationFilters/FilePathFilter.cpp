#include "mull/MutationFilters/FilePathFilter.h"

#include "mull/MutationPoint.h"

#include <cassert>
#include <iostream>

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

bool FilePathFilter::shouldSkip(const std::string &sourceFilePath) const {
  for (auto r : filters) {
    if (std::regex_search(sourceFilePath, r)) {
      ///std::cout << "FilePathFilter.shouldSkip: " << sourceFilePath << "\n";
      return true;
    }
  }

  return false;
}

void FilePathFilter::exclude(const std::string &filter) {
  std::cout << "\tFilePathFilter::exclude: " << filter << "\n";

  filters.push_back(std::regex(filter, std::regex::grep));
}

