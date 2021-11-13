#pragma once

namespace mull {

class Result;

enum class ReporterKind { IDE, SQLite, Elements, Patches, GithubAnnotations };

class Reporter {
public:
  virtual void reportResults(const Result &result) = 0;
  virtual ~Reporter() = default;
};

} // namespace mull
