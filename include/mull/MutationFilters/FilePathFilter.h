#pragma once

#include "MutationFilter.h"

#include <regex>
#include <vector>

namespace mull {

class FilePathFilter : public MutationFilter {
public:
  bool shouldSkip(MutationPoint *point) override;
  std::string name() override;
  void exclude(const std::string &filter);

private:
  std::vector<std::regex> filters;
};

}