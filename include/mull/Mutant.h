#pragma once

#include "mull/Mutators/MutatorKind.h"
#include "mull/SourceLocation.h"
#include <memory>
#include <string>
#include <vector>

namespace mull {

class MutationPoint;
struct SourceLocation;

class Mutant {
public:
  Mutant(std::string identifier, std::string mutatorIdentifier, SourceLocation sourceLocation,
         SourceLocation endLocation, std::string replacement);

  const std::string &getIdentifier() const;
  const SourceLocation &getSourceLocation() const;
  const SourceLocation &getEndLocation() const;
  const std::string &getMutatorIdentifier() const;
  const std::string &getReplacement() const;
  void setCovered(bool covered);
  bool isCovered() const;

  /// needed by AST search
  void setMutatorKind(MutatorKind kind);
  MutatorKind getMutatorKind() const;

private:
  std::string identifier;
  std::string mutatorIdentifier;
  SourceLocation sourceLocation;
  SourceLocation endLocation;
  bool covered;
  std::string replacement;

  /// Needed by AST search
  MutatorKind mutatorKind;
};

struct MutantComparator {
  bool operator()(std::unique_ptr<Mutant> &lhs, std::unique_ptr<Mutant> &rhs);
  bool operator()(Mutant &lhs, Mutant &rhs);
};

} // namespace mull
