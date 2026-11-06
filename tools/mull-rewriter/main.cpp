// mull-rewriter: rewrites the sources of a program into a mutant schema.
//
// Every mutated expression becomes a switch between the original and its mutants, each mutant
// enabled by an environment variable named after its identifier, like in the IR frontend:
//
//   return a + b;  ->  return (__mull_on(MULL_ID_0) ? (a - b) : (a + b));
//
// The rewritten program compiles with a plain compiler and mull-runner runs it as usual.
//
// Workflow:
//   1. build with the IR frontend, get the mutants: mull-runner --dry-run --reporters SQLite
//   2. mull-rewriter --report report.sqlite -p <dir with compile_commands.json>
//   3. build the rewritten sources from the output directory (or in place with --in-place)

#include "Mutants.h"
#include "Schema.h"

#if __has_include(<clang/Options/OptionUtils.h>)
#include <clang/Options/OptionUtils.h>
#else
#include <clang/Driver/Driver.h>
#endif
#include <clang/Tooling/ArgumentsAdjusters.h>
#include <clang/Tooling/CompilationDatabase.h>
#include <clang/Tooling/Tooling.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/Path.h>
#include <llvm/Support/Program.h>
#include <llvm/Support/raw_ostream.h>

#include <llvm/ADT/DenseSet.h>
#include <llvm/ADT/StringSet.h>

using namespace clang::tooling;
using namespace mull_rewriter;

static llvm::cl::OptionCategory Category("mull-rewriter options");

static llvm::cl::opt<std::string> ReportPath("report", llvm::cl::desc("Mull SQLite report"),
                                             llvm::cl::value_desc("path"), llvm::cl::Required,
                                             llvm::cl::cat(Category));

static llvm::cl::opt<std::string> BuildPath("p",
                                            llvm::cl::desc("Directory with compile_commands.json"),
                                            llvm::cl::value_desc("dir"), llvm::cl::init("."),
                                            llvm::cl::cat(Category));

static llvm::cl::list<std::string>
    ExtraArgs("extra-arg", llvm::cl::desc("Additional argument for every compile command"),
              llvm::cl::cat(Category));

static llvm::cl::list<std::string> Sources(llvm::cl::Positional,
                                           llvm::cl::desc("[<source>...], all by default"),
                                           llvm::cl::cat(Category));

static llvm::cl::opt<std::string> OutputDir(
    "output-dir", llvm::cl::desc("Directory for the rewritten files, paths relative to --root"),
    llvm::cl::value_desc("dir"), llvm::cl::init("mull-rewritten"), llvm::cl::cat(Category));

static llvm::cl::opt<std::string>
    Root("root", llvm::cl::desc("Root of the sources, the current directory by default"),
         llvm::cl::value_desc("dir"), llvm::cl::init("."), llvm::cl::cat(Category));

static llvm::cl::opt<bool>
    InPlace("in-place", llvm::cl::desc("Rewrite the files in place instead of into --output-dir"),
            llvm::cl::cat(Category));

static std::string realPath(llvm::StringRef path) {
  llvm::SmallString<256> real;
  return llvm::sys::fs::real_path(path, real) ? path.str() : std::string(real);
}

static std::string resourcesPathOf(llvm::StringRef compiler) {
#if __has_include(<clang/Options/OptionUtils.h>)
  return clang::GetResourcesPath(compiler);
#else
  return clang::driver::Driver::GetResourcesPath(compiler);
#endif
}

/// The resource directory of the command's compiler, the tool's own would be next to the tool
static ArgumentsAdjuster resourceDirAdjuster() {
  return [](const CommandLineArguments &arguments, llvm::StringRef) {
    for (const std::string &argument : arguments) {
      if (llvm::StringRef(argument).starts_with("-resource-dir")) {
        return arguments;
      }
    }
    llvm::ErrorOr<std::string> compiler = llvm::sys::findProgramByName(arguments.front());
    if (!compiler) {
      return arguments;
    }
    CommandLineArguments adjusted = arguments;
    adjusted.insert(adjusted.begin() + 1, "-resource-dir=" + resourcesPathOf(realPath(*compiler)));
    return adjusted;
  };
}

static int parse(const CompilationDatabase &database, const std::vector<std::string> &files,
                 SiteCollector &collector) {
  if (files.empty()) {
    return 0;
  }
  ClangTool tool(database, files);
  tool.appendArgumentsAdjuster(getInsertArgumentAdjuster(ExtraArgs, ArgumentInsertPosition::END));
  tool.appendArgumentsAdjuster(resourceDirAdjuster());
  clang::ast_matchers::MatchFinder finder;
  collector.registerMatchers(finder);
  return tool.run(newFrontendActionFactory(&finder).get());
}

static std::string destinationOf(const std::string &path) {
  if (InPlace) {
    return path;
  }
  llvm::StringRef relative(path);
  if (!relative.consume_front(realPath(Root) + "/")) {
    relative = relative.ltrim('/');
  }
  llvm::SmallString<256> destination(OutputDir);
  llvm::sys::path::append(destination, relative);
  return std::string(destination);
}

static bool writeFile(const std::string &path, const std::string &text) {
  std::error_code error = llvm::sys::fs::create_directories(llvm::sys::path::parent_path(path));
  if (!error) {
    llvm::raw_fd_ostream stream(path, error);
    stream << text;
  }
  if (error) {
    llvm::errs() << "error: cannot write '" << path << "': " << error.message() << "\n";
  }
  return !error;
}

int main(int argc, const char **argv) {
  llvm::cl::HideUnrelatedOptions(Category);
  llvm::cl::ParseCommandLineOptions(argc, argv, "Rewrites sources into a Mull mutant schema\n");

  std::string databaseError;
  std::unique_ptr<CompilationDatabase> database =
      CompilationDatabase::loadFromDirectory(BuildPath, databaseError);
  if (!database) {
    llvm::errs() << "error: " << databaseError << "\n";
    return 1;
  }

  std::vector<Mutant> mutants;
  if (std::optional<std::string> error = readMutants(ReportPath, mutants)) {
    llvm::errs() << "error: " << *error << "\n";
    return 1;
  }

  // Files with mutants first, the others only if mutants in headers are still missing
  llvm::StringSet<> mutatedFiles;
  for (const Mutant &mutant : mutants) {
    mutatedFiles.insert(mutant.file);
  }
  std::vector<std::string> sources(Sources.begin(), Sources.end());
  if (sources.empty()) {
    sources = database->getAllFiles();
  }
  std::vector<std::string> mutatedSources;
  std::vector<std::string> otherSources;
  for (const std::string &source : sources) {
    (mutatedFiles.contains(realPath(source)) ? mutatedSources : otherSources).push_back(source);
  }

  SiteCollector collector(mutants);
  int status = parse(*database, mutatedSources, collector);
  bool missing = std::any_of(mutants.begin(), mutants.end(), [&](const Mutant &mutant) {
    return targetOf(mutant.mutator) && !collector.isPlaced(mutant) &&
           !collector.isUnrepresentable(mutant);
  });
  if (missing) {
    status |= parse(*database, otherSources, collector);
  }
  collector.finish();
  if (status != 0) {
    llvm::errs() << "warning: some sources failed to parse, their mutants may be missing\n";
  }

  llvm::DenseSet<const Mutant *> dropped;
  unsigned fileIndex = 0;
  for (const auto &entry : collector.sites()) {
    std::string path = entry.getKey().str();
    const FileSites &sites = entry.getValue();
    auto buffer = llvm::MemoryBuffer::getFile(path);
    if (!buffer) {
      llvm::errs() << "error: cannot read '" << path << "'\n";
      return 1;
    }
    std::string source = (*buffer)->getBuffer().str();
    if (llvm::StringRef(source).starts_with(RewrittenMarker)) {
      llvm::errs() << "error: '" << path << "' is already rewritten\n";
      return 1;
    }
    RenderedFile rendered = renderSchema(path, source, sites, fileIndex++);
    dropped.insert(rendered.dropped.begin(), rendered.dropped.end());
    if (!writeFile(destinationOf(path), rendered.text)) {
      return 1;
    }
  }

  unsigned rewritten = 0;
  for (const Mutant &mutant : mutants) {
    if (!targetOf(mutant.mutator)) {
      llvm::outs() << "Not rewritten (unsupported mutator): " << mutant.identifier << "\n";
    } else if (collector.isUnrepresentable(mutant)) {
      llvm::outs() << "Not rewritten (no source equivalent): " << mutant.identifier << "\n";
    } else if (!collector.isPlaced(mutant)) {
      llvm::outs() << "Not rewritten (expression not found, e.g. in a macro): " << mutant.identifier
                   << "\n";
    } else if (dropped.contains(&mutant)) {
      llvm::outs() << "Not rewritten (overlaps another mutant): " << mutant.identifier << "\n";
    } else {
      rewritten++;
    }
  }
  llvm::outs() << "Rewrote " << rewritten << " of " << mutants.size() << " mutants in "
               << collector.sites().size() << " files\n";
  return 0;
}
