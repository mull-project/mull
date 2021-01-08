#pragma once

#include <memory>
#include <string>
#include <vector>

namespace mull {

class MutationPoint;
struct SourceLocation;

class Mutant {
public:
  Mutant(std::string identifier, std::vector<MutationPoint *> points);

  const std::string &getIdentifier() const;
  const std::vector<MutationPoint *> &getMutationPoints() const;
  const SourceLocation &getSourceLocation() const;
  const std::string &getDiagnostics() const;
  const std::string &getReplacement() const;
  const std::string &getMutatorIdentifier() const;

private:
  std::string identifier;
  std::string mutatorIdentifier;
  std::vector<MutationPoint *> points;
};

struct MutantComparator {
  bool operator()(std::unique_ptr<Mutant> &lhs, std::unique_ptr<Mutant> &rhs);
  bool operator()(Mutant &lhs, Mutant &rhs);
};

} // namespace mull
