#include "Schema.h"

#include <clang/AST/Expr.h>
#include <clang/ASTMatchers/ASTMatchers.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Lex/Lexer.h>
#include <clang/Rewrite/Core/Rewriter.h>
#include <llvm/Support/raw_ostream.h>

#include <algorithm>

using namespace clang;
using namespace clang::ast_matchers;

namespace mull_rewriter {

const char *const RewrittenMarker = "/* Rewritten by mull-rewriter */";

/// `__mull_on(id, encoding)` checks the environment like the IR frontend does, and puts the
/// encoding into the section mull-runner reads the mutants from
static const char *const Prelude = R"(#include <stdlib.h>
#ifndef __mull_on
#if defined(__APPLE__)
#define __MULL_MUTANTS_SECTION "__mull,.mull_mutants"
#else
#define __MULL_MUTANTS_SECTION ".mull_mutants"
#endif
#define __mull_on(id, encoding)                                                                    \
  ({                                                                                               \
    __attribute__((used, section(__MULL_MUTANTS_SECTION))) static const char __mull_mutant[] =     \
        encoding;                                                                                  \
    getenv(id) != 0;                                                                               \
  })
#endif
)";

static std::string locationKey(llvm::StringRef path, unsigned line, unsigned column) {
  return (path + ":" + llvm::Twine(line) + ":" + llvm::Twine(column)).str();
}

SiteCollector::SiteCollector(const std::vector<Mutant> &mutants) {
  for (const Mutant &mutant : mutants) {
    mutantsByLocation[locationKey(mutant.file, mutant.beginLine, mutant.beginColumn)].push_back(
        &mutant);
  }
}

void SiteCollector::registerMatchers(MatchFinder &finder) {
  // Instantiations share the source text and the mutants of their template. The schema uses
  // statement expressions, which only work inside functions.
  auto rewritable = allOf(unless(isInTemplateInstantiation()), hasAncestor(functionDecl()));
  finder.addMatcher(binaryOperator(rewritable).bind("binary"), this);
  finder.addMatcher(unaryOperator(rewritable).bind("unary"), this);
  finder.addMatcher(callExpr(rewritable).bind("call"), this);
  finder.addMatcher(
      varDecl(hasInitializer(expr()), unless(isInstantiated()), hasAncestor(functionDecl()))
          .bind("variable"),
      this);
}

void SiteCollector::run(const MatchFinder::MatchResult &result) {
  const ASTContext &context = *result.Context;
  if (auto binary = result.Nodes.getNodeAs<BinaryOperator>("binary")) {
    place(binary, NodeKind::Binary, binary->getOpcodeStr(), binary->getOperatorLoc(), context);
    if (binary->getOpcode() == BO_Assign) {
      place(binary->getRHS(),
            NodeKind::Assignment,
            "=",
            binary->getOperatorLoc(),
            context,
            binary->getLHS()->getType());
    }
  } else if (auto unary = result.Nodes.getNodeAs<UnaryOperator>("unary")) {
    place(unary,
          unary->isPostfix() ? NodeKind::UnaryPostfix : NodeKind::UnaryPrefix,
          UnaryOperator::getOpcodeStr(unary->getOpcode()),
          unary->getOperatorLoc(),
          context);
  } else if (auto call = result.Nodes.getNodeAs<CallExpr>("call")) {
    place(call, NodeKind::Call, "", call->getBeginLoc(), context);
  } else if (auto variable = result.Nodes.getNodeAs<VarDecl>("variable")) {
    const Expr *init = variable->getInit();
    place(init, NodeKind::Initializer, "", init->getBeginLoc(), context, variable->getType());
  }
}

static std::string realPath(const SourceManager &sourceManager, FileID file) {
  OptionalFileEntryRef entry = sourceManager.getFileEntryRefForID(file);
  if (!entry) {
    return "";
  }
  llvm::StringRef real = entry->getFileEntry().tryGetRealPathName();
  return real.empty() ? entry->getName().str() : real.str();
}

static bool fits(const Mutant &mutant, NodeKind kind, llvm::StringRef op, const Stmt *node) {
  std::optional<MutatorTarget> target = targetOf(mutant.mutator);
  if (!target || target->kind != kind || target->op != op) {
    return false;
  }
  if (kind != NodeKind::Call) {
    return true;
  }
  bool returnsVoid = cast<CallExpr>(node)->getType()->isVoidType();
  return returnsVoid == (mutant.mutator == "cxx_remove_void_call");
}

/// The value a constant mutant stores
static std::string valueOf(const Mutant &mutant, NodeKind kind, QualType type,
                           const ASTContext &context) {
  llvm::StringRef value = llvm::StringRef(mutant.replacement).trim();
  if (kind == NodeKind::Assignment) {
    // The replacement patches the '=' token: "= <value>;"
    value.consume_front("=");
    value.consume_back(";");
    value = value.trim();
  }
  std::string text = value.str();
  if (!context.getLangOpts().Bool && (text == "true" || text == "false")) {
    text = text == "true" ? "1" : "0";
  }
  // C++ does not convert integers to enums implicitly
  if (type->isEnumeralType()) {
    text = "(" + type.getUnqualifiedType().getAsString(context.getPrintingPolicy()) + ")(" + text +
           ")";
  }
  return text;
}

void SiteCollector::place(const Stmt *node, NodeKind kind, llvm::StringRef op, SourceLocation key,
                          const ASTContext &context, QualType valueType) {
  const SourceManager &sourceManager = context.getSourceManager();
  const LangOptions &langOptions = context.getLangOpts();
  // Rewriting a macro body would change every expansion of it
  if (key.isInvalid() || key.isMacroID()) {
    return;
  }
  FileID file = sourceManager.getFileID(key);
  std::string path = realPath(sourceManager, file);
  auto candidates = mutantsByLocation.find(locationKey(
      path, sourceManager.getSpellingLineNumber(key), sourceManager.getSpellingColumnNumber(key)));
  if (candidates == mutantsByLocation.end()) {
    return;
  }

  CharSourceRange range = Lexer::makeFileCharRange(
      CharSourceRange::getTokenRange(node->getSourceRange()), sourceManager, langOptions);
  if (range.isInvalid() || sourceManager.getFileID(range.getBegin()) != file) {
    return;
  }
  unsigned siteBegin = sourceManager.getFileOffset(range.getBegin());
  unsigned siteEnd = sourceManager.getFileOffset(range.getEnd());

  for (const Mutant *mutant : candidates->second) {
    if (placed.contains(mutant->identifier) || !fits(*mutant, kind, op, node)) {
      continue;
    }
    PlacedMutant placedMutant{ mutant, siteBegin, siteEnd, "" };
    if (kind == NodeKind::Initializer || kind == NodeKind::Assignment) {
      placedMutant.value = valueOf(*mutant, kind, valueType, context);
    } else {
      placedMutant.begin = sourceManager.getFileOffset(
          sourceManager.translateLineCol(file, mutant->beginLine, mutant->beginColumn));
      placedMutant.end = sourceManager.getFileOffset(
          sourceManager.translateLineCol(file, mutant->endLine, mutant->endColumn));
      if (placedMutant.begin < siteBegin || placedMutant.end > siteEnd ||
          placedMutant.begin > placedMutant.end) {
        continue;
      }
    }
    Site &site = sitesByFile[path][{ siteBegin, siteEnd }];
    site.begin = siteBegin;
    site.end = siteEnd;
    site.mutants.push_back(placedMutant);
    placed.insert(mutant->identifier);
  }
}

static std::string quoted(llvm::StringRef text) {
  std::string result;
  llvm::raw_string_ostream stream(result);
  stream << '"';
  stream.write_escaped(text);
  stream << '"';
  return result;
}

/// Whether [begin, end) cuts through `other` instead of containing or missing it
static bool cuts(unsigned begin, unsigned end, std::pair<unsigned, unsigned> other) {
  bool overlaps = other.first < end && begin < other.second;
  bool contains = begin <= other.first && other.second <= end;
  return overlaps && !contains;
}

RenderedFile renderSchema(const std::string &path, const std::string &source,
                          const FileSites &sites) {
  RenderedFile output;
  SourceManagerForFile file(path, source);
  SourceManager &sourceManager = file.get();
  FileID id = sourceManager.getMainFileID();
  Rewriter rewriter(sourceManager, LangOptions());
  auto text = [&](unsigned begin, unsigned end) -> std::string {
    if (begin == end) {
      return "";
    }
    SourceLocation start = sourceManager.getLocForStartOfFile(id);
    return rewriter.getRewrittenText(
        CharSourceRange::getCharRange(start.getLocWithOffset(begin), start.getLocWithOffset(end)));
  };

  // Innermost first: an enclosing site takes the already rewritten text of the inner ones
  std::vector<const Site *> ordered;
  for (auto &entry : sites) {
    ordered.push_back(&entry.second);
  }
  std::stable_sort(ordered.begin(), ordered.end(), [](const Site *lhs, const Site *rhs) {
    return lhs->end - lhs->begin < rhs->end - rhs->begin;
  });

  std::vector<std::pair<unsigned, unsigned>> rewritten;
  for (const Site *site : ordered) {
    auto cutBy = [&](unsigned begin, unsigned end) {
      return std::any_of(
          rewritten.begin(), rewritten.end(), [&](auto other) { return cuts(begin, end, other); });
    };
    std::string schema;
    for (const PlacedMutant &placed : site->mutants) {
      // Inner sites within the mutated text are replaced along with it
      if (cutBy(site->begin, site->end) || cutBy(placed.begin, placed.end)) {
        output.dropped.push_back(placed.mutant);
        continue;
      }
      std::string mutated =
          placed.value.empty()
              ? text(site->begin, placed.begin) + placed.mutant->replacement +
                    text(placed.end, site->end)
              : "((void)(" + text(site->begin, site->end) + "), " + placed.value + ")";
      if (llvm::StringRef(mutated).trim().empty()) {
        // E.g. a removed void call
        mutated = "(void)0";
      }
      const Mutant &mutant = *placed.mutant;
      schema += "__mull_on(" + quoted(mutant.identifier) + ", " +
                quoted(mutant.identifier + ":" + mutant.replacement) + ") ? (" + mutated + ") : ";
    }
    if (schema.empty()) {
      continue;
    }
    std::string original = text(site->begin, site->end);
    SourceLocation start = sourceManager.getLocForStartOfFile(id);
    rewriter.ReplaceText(CharSourceRange::getCharRange(start.getLocWithOffset(site->begin),
                                                       start.getLocWithOffset(site->end)),
                         "(" + schema + "(" + original + "))");
    rewritten.emplace_back(site->begin, site->end);
  }

  std::string body = source;
  if (const auto *buffer = rewriter.getRewriteBufferFor(id)) {
    body = std::string(buffer->begin(), buffer->end());
  }
  output.text = std::string(RewrittenMarker) + "\n" + Prelude + body;
  return output;
}

} // namespace mull_rewriter
