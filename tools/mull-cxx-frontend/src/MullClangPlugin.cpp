#include "ASTInstrumentation.h"
#include "ASTMutationsSearchVisitor.h"
#include "ASTNodeFactory.h"
#include "MullASTMutator.h"
#include "MutationMap.h"
#include "mull/Config/Configuration.h"
#include "mull/Diagnostics/Diagnostics.h"

#include <clang/AST/AST.h>
#include <clang/AST/ASTConsumer.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendPluginRegistry.h>
#include <clang/Sema/Sema.h>
#include <clang/Sema/SemaConsumer.h>
#include <llvm/Support/raw_ostream.h>

using namespace clang;
using namespace llvm;

namespace mull {
namespace cxx {

class MullASTConsumer : public ASTConsumer {
  CompilerInstance &instance;
  std::unique_ptr<MullASTMutator> astMutator;
  MutationMap mutationMap;

public:
  MullASTConsumer(CompilerInstance &instance, const MutationMap mutationMap)
      : instance(instance), astMutator(nullptr), mutationMap(mutationMap) {}

  void Initialize(ASTContext &Context) override {
    ASTConsumer::Initialize(Context);
  }

  /// This function can be considered a main() function of the
  /// mull-cxx-frontend plugin. This method is called multiple times by
  /// clang::ParseAST() for each declaration when it's finished being parsed.
  /// For each found function declaration below, a two-pass approach is used:
  /// 1) First all mutation points are found in the function declaration by the
  /// recursive AST visitor class ASTMutationsSearchVisitor.
  /// 2) For each mutation point, the mutations are performed on the Clang AST
  /// level. The mutation is performed by the higher-level MullASTMutator class
  /// which class to the lower-level ClangASTMutator class.
  bool HandleTopLevelDecl(DeclGroupRef DG) override {
    /// Could be a better place to create this. But at Initialize(), getSema()
    /// hits an internal assert because it is not initialized yet at that time.
    if (!astMutator) {
      astMutator = std::make_unique<MullASTMutator>(instance.getASTContext(), instance.getSema());
      astMutator->instrumentTranslationUnit();
    }

    for (DeclGroupRef::iterator I = DG.begin(), E = DG.end(); I != E; ++I) {
      if ((*I)->getKind() != Decl::Function) {
        continue;
      }

      FunctionDecl *f = static_cast<FunctionDecl *>(*I);
      if (f->getDeclName().getAsString() == "main") {
        continue;
      }

      clang::SourceLocation functionLocation = f->getLocation();
      if (instance.getSourceManager().isInSystemHeader(functionLocation)) {
        continue;
      }
      std::string sourceFilePath = instance.getSourceManager().getFilename(functionLocation).str();
      if (sourceFilePath.find("include/gtest") != std::string::npos) {
        continue;
      }
      ASTMutationsSearchVisitor visitor(instance.getASTContext(), mutationMap);
      errs() << "HandleTopLevelDecl: Looking at function: " << f->getDeclName() << "\n";
      visitor.TraverseFunctionDecl(f);

      for (auto &foundMutation : visitor.getAstMutations()) {
        foundMutation->performMutation(*astMutator);
      }
    }

    return true;
  }

  // This method is the last to be called when all declarations have already
  // been called on with HandleTopLevelDecl(). At this point, it is possible to
  // visualize the final mutated AST tree.
  void HandleTranslationUnit(ASTContext &context) override {
    // The following is useful for debugging mutations:
    // context.getTranslationUnitDecl()->print(llvm::errs(), 2);
    // context.getTranslationUnitDecl()->dump();
    // exit(1);
  }
};

class MullAction : public PluginASTAction {
  MutationMap mutationMap;

protected:
  std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI, llvm::StringRef) override {
    return std::make_unique<MullASTConsumer>(CI, mutationMap);
  }

  bool ParseArgs(const CompilerInstance &CI, const std::vector<std::string> &args) override {
    mull::Diagnostics diagnostics;
    std::string configPath = mull::Configuration::findConfig(diagnostics);
    Configuration configuration;
    if (!configPath.empty()) {
      configuration = mull::Configuration::loadFromDisk(diagnostics, configPath);
    }
    for (const std::string &mutator : configuration.mutators) {
      mutationMap.addMutation(mutator);
    }
    mutationMap.setDefaultMutationsIfNotSpecified();
    return true;
  }

  PluginASTAction::ActionType getActionType() override {
    /// Note: AddBeforeMainAction is the only option when mutations have effect.
    return AddBeforeMainAction;
  }
};

} // namespace cxx
} // namespace mull

static FrontendPluginRegistry::Add<mull::cxx::MullAction> X("mull-cxx-frontend",
                                                            "Mull: Prepare mutations");
