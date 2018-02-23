#pragma once

namespace mull {

class Result;
class Config;
class Metrics;

class Reporter {
public:
  virtual void reportResults(const Result &result,
                             const Config &config,
                             const Metrics &metrics) = 0;
  virtual ~Reporter() = default;
};

}

