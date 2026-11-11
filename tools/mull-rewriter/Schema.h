#pragma once

#include "Mutants.h"

#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <llvm/ADT/DenseMap.h>
#include <llvm/ADT/StringMap.h>
#include <llvm/ADT/StringSet.h>

#include <string>
#include <vector>

namespace mull_rewriter {

/// A mutant placed in a file: the byte range it replaces
struct PlacedMutant {
  const Mutant *mutant;
  unsigned begin;
  unsigned end;
  /// The text replacing the range
  std::string replacement;
  /// If set, the value the site evaluates to instead of the replacement
  std::string value = "";
  /// The original is still evaluated for its effects
  bool evaluated = false;
  /// A C-style cast of the value or `__mull_as` for the type of the original, if needed
  std::string cast = "";
  /// Byte ranges of the object and arguments of a replaced call, still evaluated like in the IR
  std::vector<std::pair<unsigned, unsigned>> operands;
};

/// An AST node whose source text gets switched between the original and its mutants
struct Site {
  unsigned begin;
  unsigned end;
  std::vector<PlacedMutant> mutants;
  /// A cast of the original, the type of an implicit conversion would not match the mutants
  std::string originalCast = "";
};

/// Sites of one file, keyed by their byte range
using FileSites = llvm::DenseMap<std::pair<unsigned, unsigned>, Site>;

/// Finds the AST node of each mutant, merging the sites found in several translation units
class SiteCollector : public clang::ast_matchers::MatchFinder::MatchCallback {
public:
  explicit SiteCollector(const std::vector<Mutant> &mutants);

  void registerMatchers(clang::ast_matchers::MatchFinder &finder);
  void run(const clang::ast_matchers::MatchFinder::MatchResult &result) override;
  /// Drops the mutants found unrepresentable in some instantiation, call after the matching
  void finish();

  const llvm::StringMap<FileSites> &sites() const {
    return sitesByFile;
  }
  bool isPlaced(const Mutant &mutant) const {
    return placed.contains(mutant.identifier);
  }
  /// The mutant fits a node, but has no source equivalent
  bool isUnrepresentable(const Mutant &mutant) const {
    return unrepresentable.contains(mutant.identifier);
  }

private:
  /// Operator mutants of builtin instantiations have no source equivalent for overloaded ones
  void poisonOperator(clang::SourceLocation key, const clang::ASTContext &context);
  void place(const clang::Stmt *node, NodeKind kind, llvm::StringRef op, clang::SourceLocation key,
             const clang::ASTContext &context, clang::QualType valueType = clang::QualType(),
             bool deduced = false);

  /// Mutants by "<real path>:<line>:<column>" of their begin
  llvm::StringMap<std::vector<const Mutant *>> mutantsByLocation;
  llvm::StringMap<FileSites> sitesByFile;
  llvm::StringSet<> placed;
  llvm::StringSet<> unrepresentable;
};

/// The schema of a file: every site switches between its mutants at run time
struct RenderedFile {
  std::string text;
  /// Mutants that could not be rendered
  std::vector<const Mutant *> dropped;
};
/// `fileIndex` names the per-file guard, unique among the rewritten files
RenderedFile renderSchema(const std::string &path, const std::string &source,
                          const FileSites &sites, unsigned fileIndex);

/// The first line of every rewritten file
extern const char *const RewrittenMarker;

} // namespace mull_rewriter
