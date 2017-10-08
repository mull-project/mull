#pragma once

#include "TestResult.h"

namespace mull {

class MutationPoint;

class IDEDiagnostics {
public:
  virtual void report(MutationPoint *mutationPoint, ExecutionStatus Status) = 0;
  virtual ~IDEDiagnostics() {};
};

class NormalIDEDiagnostics: public IDEDiagnostics {
public:
  void report(MutationPoint *mutationPoint, ExecutionStatus Status);
};

class NullIDEDiagnostics: public IDEDiagnostics {
public:
  void report(MutationPoint *mutationPoint, ExecutionStatus Status) {}
};

}
