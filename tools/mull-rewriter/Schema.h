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
  /// If set, the value the site evaluates to instead, the site is still evaluated for its effects
  std::string value;
};

/// An AST node whose source text gets switched between the original and its mutants
struct Site {
  unsigned begin;
  unsigned end;
  std::vector<PlacedMutant> mutants;
};

/// Sites of one file, keyed by their byte range
using FileSites = llvm::DenseMap<std::pair<unsigned, unsigned>, Site>;

/// Finds the AST node of each mutant, merging the sites found in several translation units
class SiteCollector : public clang::ast_matchers::MatchFinder::MatchCallback {
public:
  explicit SiteCollector(const std::vector<Mutant> &mutants);

  void registerMatchers(clang::ast_matchers::MatchFinder &finder);
  void run(const clang::ast_matchers::MatchFinder::MatchResult &result) override;

  const llvm::StringMap<FileSites> &sites() const {
    return sitesByFile;
  }
  bool isPlaced(const Mutant &mutant) const {
    return placed.contains(mutant.identifier);
  }

private:
  void place(const clang::Stmt *node, NodeKind kind, llvm::StringRef op, clang::SourceLocation key,
             const clang::ASTContext &context, clang::QualType valueType = clang::QualType());

  /// Mutants by "<real path>:<line>:<column>" of their begin
  llvm::StringMap<std::vector<const Mutant *>> mutantsByLocation;
  llvm::StringMap<FileSites> sitesByFile;
  llvm::StringSet<> placed;
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
