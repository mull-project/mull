#pragma once

#include "Config/RawConfig.h"
#include "ExecutionResult.h"

namespace mull {

class MutationPoint;

class IDEDiagnostics {
public:
  virtual void report(MutationPoint *mutationPoint, bool killed) = 0;
  virtual ~IDEDiagnostics() {};
};

class NormalIDEDiagnostics: public IDEDiagnostics {
  RawConfig::Diagnostics diagnostics;
public:
  NormalIDEDiagnostics(RawConfig::Diagnostics diagnostics) :
    diagnostics(diagnostics) {}

  void report(MutationPoint *mutationPoint, bool killed);
};

class NullIDEDiagnostics: public IDEDiagnostics {
public:
  void report(MutationPoint *mutationPoint, bool killed) {}
};

}
