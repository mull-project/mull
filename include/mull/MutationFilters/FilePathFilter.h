#pragma once

#include "MutationFilter.h"

#include <regex>
#include <vector>

namespace mull {

class FilePathFilter : public MutationFilter {
public:
  bool shouldSkip(MutationPoint *point) override;
  bool shouldSkip(const std::string &sourceFilePath) const;
  std::string name() override;

  void exclude(const std::string &filter);

private:
  std::vector<std::regex> filters;
};

}