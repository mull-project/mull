#include "mull/Mutant.h"

#include "mull/SourceLocation.h"
#include <utility>

using namespace mull;

Mutant::Mutant(std::string identifier, std::string mutatorIdentifier, SourceLocation sourceLocation,
               SourceLocation endLocation)
    : identifier(std::move(identifier)), mutatorIdentifier(std::move(mutatorIdentifier)),
      sourceLocation(std::move(sourceLocation)), endLocation(std::move(endLocation)),
      covered(false), mutatorKind(MutatorKind::InvalidKind) {}

const std::string &Mutant::getIdentifier() const {
  return identifier;
}

const SourceLocation &Mutant::getSourceLocation() const {
  return sourceLocation;
}

const SourceLocation &Mutant::getEndLocation() const {
  return endLocation;
}

const std::string &Mutant::getMutatorIdentifier() const {
  return mutatorIdentifier;
}

bool Mutant::isCovered() const {
  return covered;
}

void Mutant::setMutatorKind(MutatorKind kind) {
  mutatorKind = kind;
}

MutatorKind Mutant::getMutatorKind() const {
  return mutatorKind;
}

void Mutant::setCovered(bool cover) {
  covered = cover;
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
