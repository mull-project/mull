#pragma once

namespace mull {

class Result;
class RawConfig;
class Metrics;

class Reporter {
public:
  virtual void reportResults(const Result &result, const RawConfig &config,
                             const Metrics &metrics) = 0;
  virtual ~Reporter() = default;
};

} // namespace mull
