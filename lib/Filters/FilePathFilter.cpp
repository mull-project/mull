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
  assert(!location.isNull());
  return shouldSkip(location.filePath);
}

bool FilePathFilter::shouldSkip(const std::string &sourceFilePath) const {
  std::lock_guard<std::mutex> lock(cacheMutex);

  if (cache.count(sourceFilePath) == 0) {
    bool allow = true;

    if (!includeFilters.empty()) {
      allow = false;

      for (const auto &r : includeFilters) {
        if (std::regex_search(sourceFilePath, r)) {
          allow = true;
          break;
        }
      }
    }
    if (allow) {
      for (const auto &r : excludeFilters) {
        if (std::regex_search(sourceFilePath, r)) {
          allow = false;
          break;
        }
      }
    }

    cache[sourceFilePath] = allow;
  }

  return (! cache[sourceFilePath]);
}

std::string FilePathFilter::name() { return "file path"; }

void FilePathFilter::exclude(const std::string &filter) {
  excludeFilters.push_back(std::regex(filter, std::regex::grep));
}

void FilePathFilter::include(const std::string &filter) {
  includeFilters.push_back(std::regex(filter, std::regex::grep));
}
