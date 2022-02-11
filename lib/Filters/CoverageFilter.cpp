#include "mull/Filters/CoverageFilter.h"
#include "mull/Config/Configuration.h"
#include "mull/Diagnostics/Diagnostics.h"
#include "mull/Mutant.h"
#include <llvm/ProfileData/Coverage/CoverageMapping.h>

using namespace mull;

static std::unique_ptr<llvm::coverage::CoverageMapping>
loadCoverage(const Configuration &configuration, Diagnostics &diagnostics,
             const std::string &profileName, const std::vector<std::string> &objects) {
  if (profileName.empty()) {
    return nullptr;
  }
  std::vector<llvm::StringRef> objectFiles;
  objectFiles.reserve(objects.size());
  for (auto &object : objects) {
    objectFiles.emplace_back(object);
  }
  llvm::Expected<std::unique_ptr<llvm::coverage::CoverageMapping>> maybeMapping =
      llvm::coverage::CoverageMapping::load(objectFiles, profileName);
  if (!maybeMapping) {
    std::string error;
    llvm::raw_string_ostream os(error);
    llvm::logAllUnhandledErrors(maybeMapping.takeError(), os, "Cannot read coverage info: ");
    diagnostics.warning(os.str());
    return nullptr;
  }
  return std::move(maybeMapping.get());
}

CoverageFilter::CoverageFilter(const Configuration &configuration, Diagnostics &diagnostics,
                               const std::string &profileName,
                               const std::vector<std::string> &objects)
    : configuration(configuration) {
  std::unique_ptr<llvm::coverage::CoverageMapping> coverage =
      loadCoverage(configuration, diagnostics, profileName, objects);
  if (coverage != nullptr) {
    for (auto &it : coverage->getCoveredFunctions()) {
      std::vector<CoverageRange> ranges;
      for (auto &region : it.CountedRegions) {
        if (region.ExecutionCount == 0) {
          ranges.push_back(
              { region.LineStart, region.ColumnStart, region.LineEnd, region.ColumnEnd });
        }
      }
      for (auto &path : it.Filenames) {
        // TODO: merge adjacent ranges
        std::copy(std::begin(ranges), std::end(ranges), std::back_inserter(uncoveredRanges[path]));
      }
    }
  }

  if (configuration.debug.coverage) {
    for (auto &pair : uncoveredRanges) {
      for (auto &range : pair.second) {
        llvm::errs() << "mull-coverage: " << pair.first << ":" << range.lineBegin << ":"
                     << range.columnBegin << ":" << range.lineEnd << ":" << range.columnEnd << "\n";
      }
    }
  }
}

bool CoverageFilter::covered(Mutant *mutant) {
  // TODO: optimize lookup
  assert(mutant);
  if (uncoveredRanges.empty()) {
    return true;
  }
  auto &location = mutant->getSourceLocation();
  auto &path = location.filePath;
  if (!uncoveredRanges.count(path)) {
    return true;
  }
  for (auto &range : uncoveredRanges.at(path)) {
    bool more = (location.line > range.lineBegin) ||
                (location.line == range.lineBegin && location.column >= range.columnBegin);
    bool less = (location.line < range.lineEnd) ||
                (location.line == range.lineEnd && location.column <= range.columnEnd);
    if (more && less) {
      return false;
    }
  }
  return true;
}

bool CoverageFilter::shouldSkip(Mutant *mutant) {
  if (covered(mutant) || configuration.includeNotCovered) {
    return false;
  }
  return true;
}

std::string CoverageFilter::name() {
  return "coverage";
}
