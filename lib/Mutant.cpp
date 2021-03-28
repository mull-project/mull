#include "mull/Mutant.h"
#include "mull/MutationPoint.h"
#include "mull/SourceLocation.h"

using namespace mull;

Mutant::Mutant(std::string identifier, std::vector<MutationPoint *> points)
    : identifier(std::move(identifier)), mutatorIdentifier(points.front()->getMutatorIdentifier()),
      points(std::move(points)), mutatorKind(MutatorKind::InvalidKind) {}

const std::string &Mutant::getIdentifier() const {
  return identifier;
}

const std::vector<MutationPoint *> &Mutant::getMutationPoints() const {
  return points;
}

const SourceLocation &Mutant::getSourceLocation() const {
  return points.front()->getSourceLocation();
}

const std::string &Mutant::getDiagnostics() const {
  return points.front()->getDiagnostics();
}

const std::string &Mutant::getMutatorIdentifier() const {
  return mutatorIdentifier;
}

const std::string &Mutant::getReplacement() const {
  return points.front()->getReplacement();
}

bool Mutant::isCovered() const {
  for (MutationPoint *point : points) {
    /// Consider a mutant covered if at least one of the mutation points is covered
    if (point->isCovered()) {
      return true;
    }
  }
  return false;
}

void Mutant::setMutatorKind(MutatorKind kind) {
  mutatorKind = kind;
}

MutatorKind Mutant::getMutatorKind() const {
  return mutatorKind;
}

bool MutantComparator::operator()(std::unique_ptr<Mutant> &lhs, std::unique_ptr<Mutant> &rhs) {
  return operator()(*lhs, *rhs);
}

bool MutantComparator::operator()(Mutant &lhs, Mutant &rhs) {
  const SourceLocation &l = lhs.getSourceLocation();
  const SourceLocation &r = rhs.getSourceLocation();
  return std::tie(l.filePath, l.line, l.column, lhs.getMutatorIdentifier()) <
         std::tie(r.filePath, r.line, r.column, rhs.getMutatorIdentifier());
}
