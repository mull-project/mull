#pragma once

#include "ExecutionResult.h"
#include "mull/Config/ConfigurationOptions.h"

namespace mull {

class MutationPoint;

class IDEDiagnostics {
public:
  virtual void report(MutationPoint *mutationPoint, bool killed) = 0;
  virtual ~IDEDiagnostics() = default;
};

class NormalIDEDiagnostics : public IDEDiagnostics {
public:
  explicit NormalIDEDiagnostics(IDEDiagnosticsKind diagnostics) : diagnostics(diagnostics) {}
  void report(MutationPoint *mutationPoint, bool killed) override;

private:
  IDEDiagnosticsKind diagnostics;
};

class NullIDEDiagnostics : public IDEDiagnostics {
public:
  void report(MutationPoint *mutationPoint, bool killed) override {}
};

} // namespace mull
