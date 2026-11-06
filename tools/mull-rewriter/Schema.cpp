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

/// `__mull_on(id)` checks the environment like the IR frontend does. Evaluation at compile time,
/// e.g. of a constexpr function, always takes the original.
static const char *const Prelude = R"(#include <stdlib.h>
#ifndef __mull_on
#if defined(__cplusplus) && __has_builtin(__builtin_is_constant_evaluated)
#define __mull_on(id) (!__builtin_is_constant_evaluated() && getenv(id) != 0)
#else
#define __mull_on(id) (getenv(id) != 0)
#endif
#endif
#ifndef __mull_as
#define __mull_as(original, value) ((__typeof__(original))(value))
#endif
)";

/// The section mull-runner reads the mutants from, filled by globals at the end of the file
static const char *const SectionDefinition = R"(#ifndef MULL_MUTANTS_SECTION
#if defined(__APPLE__)
#define MULL_MUTANTS_SECTION "__mull,.mull_mutants"
#else
#define MULL_MUTANTS_SECTION ".mull_mutants"
#endif
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
  // Instantiations too: they share non-dependent nodes with the template and type the others
  finder.addMatcher(binaryOperator().bind("binary"), this);
  finder.addMatcher(unaryOperator().bind("unary"), this);
  finder.addMatcher(callExpr().bind("call"), this);
  finder.addMatcher(cxxOperatorCallExpr().bind("overloaded"), this);
  finder.addMatcher(varDecl(unless(parmVarDecl()), hasInitializer(expr())).bind("variable"), this);
}

void SiteCollector::finish() {
  for (auto &file : sitesByFile) {
    for (auto &entry : file.getValue()) {
      llvm::erase_if(entry.second.mutants, [&](const PlacedMutant &placedMutant) {
        if (!unrepresentable.contains(placedMutant.mutant->identifier)) {
          return false;
        }
        placed.erase(placedMutant.mutant->identifier);
        return true;
      });
    }
  }
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
  } else if (auto overloaded = result.Nodes.getNodeAs<CXXOperatorCallExpr>("overloaded")) {
    poisonOperator(overloaded->getOperatorLoc(), context);
  } else if (auto call = result.Nodes.getNodeAs<CallExpr>("call")) {
    // The IR frontend reports member calls at either the object or the member name
    place(call, NodeKind::Call, "", call->getBeginLoc(), context);
    if (call->getExprLoc() != call->getBeginLoc()) {
      place(call, NodeKind::Call, "", call->getExprLoc(), context);
    }
  } else if (auto variable = result.Nodes.getNodeAs<VarDecl>("variable")) {
    const Expr *init = variable->getInit();
    if (!init) {
      return;
    }
    place(init,
          NodeKind::Initializer,
          "",
          init->getBeginLoc(),
          context,
          variable->getType(),
          variable->getType()->getContainedAutoType() != nullptr);
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

void SiteCollector::poisonOperator(SourceLocation key, const ASTContext &context) {
  const SourceManager &sourceManager = context.getSourceManager();
  if (key.isInvalid() || key.isMacroID()) {
    return;
  }
  auto candidates =
      mutantsByLocation.find(locationKey(realPath(sourceManager, sourceManager.getFileID(key)),
                                         sourceManager.getSpellingLineNumber(key),
                                         sourceManager.getSpellingColumnNumber(key)));
  if (candidates == mutantsByLocation.end()) {
    return;
  }
  for (const Mutant *mutant : candidates->second) {
    std::optional<MutatorTarget> target = targetOf(mutant->mutator);
    if (target && target->kind != NodeKind::Call && target->kind != NodeKind::Initializer) {
      unrepresentable.insert(mutant->identifier);
    }
  }
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

/// A call of a conversion operator, its source is just the converted object
static bool isConversion(const Stmt *node) {
  auto call = dyn_cast<CXXMemberCallExpr>(node);
  return call && isa_and_nonnull<CXXConversionDecl>(call->getMethodDecl());
}

/// Whether the mutant can be written in the source, e.g. IR mutates pointer differences as integers
static bool representable(NodeKind kind, const Stmt *node, QualType valueType) {
  auto scalar = [](QualType type) {
    return type->isIntegralOrEnumerationType() || type->isRealFloatingType();
  };
  switch (kind) {
  case NodeKind::Binary: {
    auto binary = cast<BinaryOperator>(node);
    return binary->getOpcode() != BO_Sub || !binary->getLHS()->getType()->isPointerType();
  }
  case NodeKind::Call: {
    QualType type = cast<CallExpr>(node)->getType();
    // The original of a conversion is cast to the type explicitly, it must have a name
    if (isConversion(node) && !type.getCanonicalType()->isBuiltinType()) {
      return false;
    }
    return type->isVoidType() || scalar(type);
  }
  case NodeKind::Initializer:
  case NodeKind::Assignment:
    // __typeof__ does not take bit-fields
    return scalar(valueType) && !cast<Expr>(node)->refersToBitField();
  default:
    return true;
  }
}

static std::string literalOf(llvm::StringRef value, const ASTContext &context) {
  std::string text = value.trim().str();
  if (!context.getLangOpts().Bool && (text == "true" || text == "false")) {
    text = text == "true" ? "1" : "0";
  }
  return text;
}

/// The cast of a value to `type`, empty if none is needed
static std::string castTo(QualType type, bool needed, const ASTContext &context) {
  if (!needed || !context.getLangOpts().CPlusPlus) {
    return "";
  }
  QualType canonical = type.getCanonicalType().getUnqualifiedType();
  if (canonical->isBuiltinType()) {
    return "(" + canonical.getAsString(context.getPrintingPolicy()) + ")";
  }
  // Names of other types may be shadowed or inaccessible at the site
  return "__mull_as";
}

/// The value a constant mutant stores
static std::string valueOf(const Mutant &mutant, NodeKind kind, const ASTContext &context) {
  llvm::StringRef value = llvm::StringRef(mutant.replacement).trim();
  if (kind == NodeKind::Assignment) {
    // The replacement patches the '=' token: "= <value>;"
    value.consume_front("=");
    value.consume_back(";");
  }
  return literalOf(value, context);
}

void SiteCollector::place(const Stmt *node, NodeKind kind, llvm::StringRef op, SourceLocation key,
                          const ASTContext &context, QualType valueType, bool deduced) {
  const SourceManager &sourceManager = context.getSourceManager();
  const LangOptions &langOptions = context.getLangOpts();
  // Left to the instantiations, the types decide whether a mutant fits
  bool dependent =
      cast<Expr>(node)->isTypeDependent() || (!valueType.isNull() && valueType->isDependentType());
  if (key.isInvalid() || dependent) {
    return;
  }
  auto fileRange = [&] {
    return Lexer::makeFileCharRange(
        CharSourceRange::getTokenRange(node->getSourceRange()), sourceManager, langOptions);
  };
  if (key.isMacroID()) {
    // Rewriting a macro body would change every expansion, a whole expansion is fine to replace
    bool whole = kind == NodeKind::Call || kind == NodeKind::Initializer;
    CharSourceRange range = whole ? fileRange() : CharSourceRange();
    if (range.isInvalid() || sourceManager.getExpansionLoc(key) != range.getBegin()) {
      return;
    }
    key = range.getBegin();
  }
  FileID file = sourceManager.getFileID(key);
  std::string path = realPath(sourceManager, file);
  auto candidates = mutantsByLocation.find(locationKey(
      path, sourceManager.getSpellingLineNumber(key), sourceManager.getSpellingColumnNumber(key)));
  if (candidates == mutantsByLocation.end()) {
    return;
  }

  CharSourceRange range = fileRange();
  if (range.isInvalid() || sourceManager.getFileID(range.getBegin()) != file) {
    return;
  }
  unsigned siteBegin = sourceManager.getFileOffset(range.getBegin());
  unsigned siteEnd = sourceManager.getFileOffset(range.getEnd());

  for (const Mutant *mutant : candidates->second) {
    if (!fits(*mutant, kind, op, node)) {
      continue;
    }
    // Instantiations of the site may differ, any of them can make it unrepresentable
    if (!representable(kind, node, valueType)) {
      unrepresentable.insert(mutant->identifier);
      continue;
    }
    if (placed.contains(mutant->identifier)) {
      continue;
    }
    // A call is replaced as a whole, the IR frontend may report just the member name
    PlacedMutant placedMutant{ mutant, siteBegin, siteEnd, mutant->replacement };
    // Keeps the type where it matters: enums, 'auto' and the type of a call
    if (kind == NodeKind::Initializer || kind == NodeKind::Assignment) {
      placedMutant.value = valueOf(*mutant, kind, context);
      placedMutant.evaluated = true;
      placedMutant.cast = castTo(valueType, deduced || valueType->isEnumeralType(), context);
    } else if (kind == NodeKind::Call) {
      if (!mutant->replacement.empty()) {
        placedMutant.value = literalOf(mutant->replacement, context);
        placedMutant.cast = castTo(cast<CallExpr>(node)->getType(), true, context);
      }
    } else {
      if (op == "!" && mutant->replacement.empty()) {
        // Keeps the type of the negation, removing it may leave e.g. a pointer
        placedMutant.replacement = "!!";
      }
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
    if (kind == NodeKind::Call && isConversion(node)) {
      site.originalCast = castTo(cast<CallExpr>(node)->getType(), true, context);
    }
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

static std::string idMacro(const Mutant &mutant) {
  return "MULL_ID_" + std::to_string(mutant.index);
}

RenderedFile renderSchema(const std::string &path, const std::string &source,
                          const FileSites &sites, unsigned fileIndex) {
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
  std::vector<const Mutant *> rendered;
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
      std::string mutated;
      if (placed.value.empty()) {
        mutated =
            text(site->begin, placed.begin) + placed.replacement + text(placed.end, site->end);
      } else {
        std::string original = text(site->begin, site->end);
        mutated = placed.value;
        if (placed.cast == "__mull_as") {
          mutated = "__mull_as((" + original + "), " + placed.value + ")";
        } else if (!placed.cast.empty()) {
          mutated = placed.cast + "(" + placed.value + ")";
        }
        if (placed.evaluated) {
          mutated = "((void)(" + original + "), " + mutated + ")";
        }
      }
      if (llvm::StringRef(mutated).trim().empty()) {
        // E.g. a removed void call
        mutated = "(void)0";
      }
      schema += "__mull_on(" + idMacro(*placed.mutant) + ") ? (" + mutated + ") : ";
      rendered.push_back(placed.mutant);
    }
    if (schema.empty()) {
      continue;
    }
    std::string original = text(site->begin, site->end);
    SourceLocation start = sourceManager.getLocForStartOfFile(id);
    rewriter.ReplaceText(CharSourceRange::getCharRange(start.getLocWithOffset(site->begin),
                                                       start.getLocWithOffset(site->end)),
                         "(" + schema + site->originalCast + "(" + original + "))");
    rewritten.emplace_back(site->begin, site->end);
  }

  std::string body = source;
  if (const auto *buffer = rewriter.getRewriteBufferFor(id)) {
    body = std::string(buffer->begin(), buffer->end());
  }
  std::sort(rendered.begin(), rendered.end(), [](const Mutant *lhs, const Mutant *rhs) {
    return lhs->index < rhs->index;
  });
  std::string ids;
  std::string globals;
  for (const Mutant *mutant : rendered) {
    ids += "#define " + idMacro(*mutant) + " " + quoted(mutant->identifier) + "\n";
    globals +=
        "__attribute__((used, section(MULL_MUTANTS_SECTION))) static const char MULL_MUTANT_" +
        std::to_string(mutant->index) + "[] = " + idMacro(*mutant) + " \":\" " +
        quoted(mutant->replacement) + ";\n";
  }
  if (!body.empty() && body.back() != '\n') {
    body += "\n";
  }
  // Guarded, a header without include guards may be included twice
  std::string guard = "MULL_MUTANTS_" + std::to_string(fileIndex);
  output.text = std::string(RewrittenMarker) + "\n" + Prelude + ids + body + "#ifndef " + guard +
                "\n#define " + guard + "\n" + SectionDefinition + globals + "#endif\n";
  return output;
}

} // namespace mull_rewriter
