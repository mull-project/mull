#include "mull/Filters/ManualFilter.h"

#include "mull/MutationPoint.h"
#include <llvm/Support/raw_ostream.h>
#include <sstream>

using namespace mull;

std::string ManualFilter::name() {
  return "manually ignored mutants";
}

ManualFilter::ManualFilter(mull::Diagnostics &diagnostics)
    : diagnostics(diagnostics), factory(diagnostics) {
  factory.init();
}

bool ManualFilter::shouldSkip(MutationPoint *point) {
  if (point->getSourceLocation().isNull()) {
    return true;
  }
  auto &location = point->getSourceLocation();
  lock.lock();
  if (ignoreRanges.count(location.filePath) == 0) {
    scanFile(location);
  }
  auto range_it = ignoreRanges.find(location.filePath);
  lock.unlock();

  for (auto &range : range_it->second) {
    if (location.line >= range.begin && location.line <= range.end) {
      return range.ignoredMutators.count(point->getMutator()->getUniqueIdentifier()) != 0;
    }
  }
  return false;
}

// TODO: dedup, this is a third copy at least
static std::vector<std::string> split(const std::string &input, char delimiter) {
  std::vector<std::string> output;
  std::string string;
  std::istringstream stream(input);
  while (std::getline(stream, string, delimiter)) {
    if (!string.empty()) {
      output.push_back(string);
    }
  }
  return output;
}

static std::vector<std::string> parseMutators(std::string line) {
  line.erase(std::remove_if(line.begin(), line.end(), isspace), line.end());
  return split(line, ',');
}

std::unordered_set<std::string> ManualFilter::expandIgnoreLine(const std::string &line,
                                                               const std::string &needle) {
  auto pos = line.find(needle);
  if (pos != std::string::npos) {
    return factory.expandMutatorGroups(parseMutators(line.substr(pos + needle.size())));
  }
  return factory.expandMutatorGroups({ "cxx_all" });
}

void ManualFilter::scanFile(const mull::SourceLocation &location) {
  std::vector<IgnoreRange> ranges;
  bool bigRange = false;
  size_t beginRange = 0;
  std::unordered_set<std::string> ignoredMutators;
  size_t i = 1;
  for (; i <= sourceManager.getNumberOfLines(location); i++) {
    std::string line = sourceManager.getLine(location.filePath, i);
    if (bigRange) {
      if (line.find("mull-on") != std::string::npos) {
        bigRange = false;
        ranges.push_back(
            IgnoreRange{ .begin = beginRange, .end = i, .ignoredMutators = ignoredMutators });
      }
    } else {
      if (line.find("mull-off") != std::string::npos) {
        beginRange = i;
        ignoredMutators = expandIgnoreLine(line, "mull-off:");
        bigRange = true;
      } else if (line.find("mull-ignore-next") != std::string::npos) {
        IgnoreRange range{ .begin = i + 1,
                           .end = i + 1,
                           .ignoredMutators = expandIgnoreLine(line, "mull-ignore-next:") };
        ranges.push_back(range);
      } else if (line.find("mull-ignore") != std::string::npos) {
        IgnoreRange range{ .begin = i,
                           .end = i,
                           .ignoredMutators = expandIgnoreLine(line, "mull-ignore:") };
        ranges.push_back(range);
      }
    }
  }
  if (bigRange) {
    ranges.push_back(
        IgnoreRange{ .begin = beginRange, .end = i, .ignoredMutators = ignoredMutators });
  }
  ignoreRanges[location.filePath] = ranges;
}
