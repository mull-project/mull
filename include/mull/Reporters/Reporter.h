#pragma once

namespace mull {

class Result;
class Metrics;

enum class ReporterKind { IDE, SQLite, Elements };

class Reporter {
public:
  virtual void reportResults(const Result &result, const Metrics &metrics) = 0;
  virtual ~Reporter() = default;
};

} // namespace mull
