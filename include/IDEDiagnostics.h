#pragma once

#include "Config.h"
#include "ExecutionResult.h"

namespace mull {

class MutationPoint;

class IDEDiagnostics {
public:
  virtual void report(MutationPoint *mutationPoint, bool killed) = 0;
  virtual ~IDEDiagnostics() {};
};

class NormalIDEDiagnostics: public IDEDiagnostics {
  Config::Diagnostics diagnostics;
public:
  NormalIDEDiagnostics(Config::Diagnostics diagnostics) :
    diagnostics(diagnostics) {}

  void report(MutationPoint *mutationPoint, bool killed);
};

class NullIDEDiagnostics: public IDEDiagnostics {
public:
  void report(MutationPoint *mutationPoint, bool killed) {}
};

}
