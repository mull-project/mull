#pragma once

#include <llvm/ADT/StringRef.h>

#include <optional>
#include <string>
#include <vector>

namespace mull_rewriter {

/// The kind of AST node a mutator applies to
enum class NodeKind {
  Binary,
  UnaryPrefix,
  UnaryPostfix,
  Call,
  /// The initializer of a variable
  Initializer,
  /// The right-hand side of '='
  Assignment
};

struct Mutant {
  std::string identifier;
  std::string mutator;
  std::string file;
  unsigned beginLine;
  unsigned beginColumn;
  unsigned endLine;
  unsigned endColumn;
  std::string replacement;
  /// Position among the mutants of the report, sorted by identifier
  unsigned index;
};

/// The node kind and operator spelling a mutator applies to, none if the mutator is not supported
struct MutatorTarget {
  NodeKind kind;
  /// Operator spelling, empty for calls and initializers
  llvm::StringRef op;
};
std::optional<MutatorTarget> targetOf(llvm::StringRef mutator);

/// Reads the distinct mutants from a Mull SQLite report, an error message on failure
std::optional<std::string> readMutants(const std::string &reportPath, std::vector<Mutant> &mutants);

} // namespace mull_rewriter
