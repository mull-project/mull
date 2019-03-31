#pragma once

#include "ExecutionResult.h"
#include "mull/Config/ConfigurationOptions.h"

namespace mull {

class MutationPoint;

class IDEDiagnostics {
public:
  virtual void report(MutationPoint *mutationPoint, bool killed) = 0;
  virtual ~IDEDiagnostics() = default;
  ;
};

class NormalIDEDiagnostics : public IDEDiagnostics {
  Diagnostics diagnostics;

public:
  explicit NormalIDEDiagnostics(Diagnostics diagnostics)
      : diagnostics(diagnostics) {}

  void report(MutationPoint *mutationPoint, bool killed) override;
};

class NullIDEDiagnostics : public IDEDiagnostics {
public:
  void report(MutationPoint *mutationPoint, bool killed) override {}
};

} // namespace mull
