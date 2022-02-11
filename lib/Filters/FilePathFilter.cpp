#include "mull/Filters/FilePathFilter.h"
#include "mull/Mutant.h"
#include "mull/MutationPoint.h"
#include "mull/SourceLocation.h"

#include <cassert>
#include <llvm/Support/raw_ostream.h>

using namespace mull;

bool FilePathFilter::shouldSkip(MutationPoint *point) {
  return shouldSkip(point->getSourceLocation());
}

bool FilePathFilter::shouldSkip(Mutant *mutant) {
  return shouldSkip(mutant->getSourceLocation());
}

bool FilePathFilter::shouldSkip(llvm::Function *function) {
  SourceLocation location = SourceLocation::locationFromFunction(function);
  return shouldSkip(location);
}

bool FilePathFilter::shouldSkip(llvm::Instruction *instruction) {
  SourceLocation location = SourceLocation::locationFromInstruction(instruction);
  return shouldSkip(location);
}

bool FilePathFilter::shouldSkip(const mull::SourceLocation &location) const {
  assert(!location.isNull());
  return shouldSkip(location.filePath);
}

bool FilePathFilter::shouldSkip(const std::string &sourceFilePath) const {
  std::lock_guard<std::mutex> lock(cacheMutex);
  std::string sourceFilePathCopy{sourceFilePath};

  if (cache.count(sourceFilePath) == 0) {
    bool allow = true;

    if (!includeFilters.empty()) {
      allow = false;

#if LLVM_VERSION_MAJOR >= 10
      for (const auto &r : includeFilters) {
#else
      for (auto &r : includeFilters) {
#endif
        if (r.match(sourceFilePathCopy)) {
          allow = true;
          break;
        }
      }
    }
    if (allow) {
#if LLVM_VERSION_MAJOR >= 10
      for (const auto &r : excludeFilters) {
#else
      for (auto &r : excludeFilters) {
#endif
        if (r.match(sourceFilePathCopy)) {
          allow = false;
          break;
        }
      }
    }

    cache[sourceFilePath] = allow;
  }

  return (!cache[sourceFilePath]);
}

std::string FilePathFilter::name() {
  return "file path";
}

std::pair<bool, std::string> FilePathFilter::exclude(const std::string &filter) {

  auto regex_filter = llvm::Regex(filter);
  std::string error;
  if (!regex_filter.isValid(error)) {
    return std::make_pair(false, std::move(error));
  }
  excludeFilters.emplace_back(std::move(regex_filter));
  return std::make_pair(true, std::string());
}

std::pair<bool, std::string> FilePathFilter::include(const std::string &filter) {
  auto regex_filter = llvm::Regex(filter);
  std::string error;
  if (!regex_filter.isValid(error)) {
    return std::make_pair(false, std::move(error));
  }
  includeFilters.emplace_back(std::move(regex_filter));
  return std::make_pair(true, std::string());
}
