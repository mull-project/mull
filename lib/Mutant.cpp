#include "mull/Mutant.h"

#include "mull/MutationPoint.h"
#include "mull/SourceLocation.h"
#include <utility>

using namespace mull;

Mutant::Mutant(std::string identifier, std::string mutatorIdentifier,
               const SourceLocation &sourceLocation)
    : identifier(std::move(identifier)), mutatorIdentifier(std::move(mutatorIdentifier)),
      sourceLocation(sourceLocation), mutatorKind(MutatorKind::InvalidKind) {}

const std::string &Mutant::getIdentifier() const {
  return identifier;
}

const SourceLocation &Mutant::getSourceLocation() const {
  return sourceLocation;
}

const std::string &Mutant::getMutatorIdentifier() const {
  return mutatorIdentifier;
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

void Mutant::setMutationPoints(std::vector<MutationPoint *> points) {
  this->points = points;
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
