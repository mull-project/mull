#pragma once

namespace mull {

class Result;
class Metrics;

class Reporter {
public:
  virtual void reportResults(const Result &result, const Metrics &metrics) = 0;
  virtual ~Reporter() = default;
};

} // namespace mull
