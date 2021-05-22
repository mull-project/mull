#include "ASTInstrumentation.h"
#include "ASTMutationsSearchVisitor.h"
#include "ASTNodeFactory.h"
#include "MullASTMutator.h"
#include "MutationMap.h"

#include <clang/AST/AST.h>
#include <clang/AST/ASTConsumer.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/FrontendPluginRegistry.h>
#include <clang/Sema/Sema.h>
#include <clang/Sema/SemaConsumer.h>
#include <llvm/Support/raw_ostream.h>

using namespace clang;
using namespace llvm;

namespace {

class MullASTConsumer : public ASTConsumer {
  CompilerInstance &Instance;
  std::unique_ptr<MullASTMutator> astMutator;
  MutationMap mutationMap;

public:
  MullASTConsumer(CompilerInstance &Instance, const MutationMap mutationMap)
      : Instance(Instance), astMutator(nullptr), mutationMap(mutationMap) {}

  void Initialize(ASTContext &Context) override {
    ASTConsumer::Initialize(Context);
  }

  bool HandleTopLevelDecl(DeclGroupRef DG) override {
    /// Should be a better place to create this. But at Initialize(), getSema() hits an internal
    /// assert.
    if (!astMutator) {
      astMutator = std::make_unique<MullASTMutator>(Instance.getASTContext(), Instance.getSema());
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
      if (Instance.getSourceManager().isInSystemHeader(functionLocation)) {
        continue;
      }
      std::string sourceFilePath = Instance.getSourceManager().getFilename(functionLocation).str();
      if (sourceFilePath.find("include/gtest") != std::string::npos) {
        continue;
      }
      ASTMutationsSearchVisitor visitor(Instance.getASTContext(), mutationMap);
      errs() << "HandleTopLevelDecl: Looking at function: " << f->getDeclName() << "\n";
      visitor.TraverseFunctionDecl(f);

      for (auto &foundMutation : visitor.getAstMutations()) {
        foundMutation->performMutation(*astMutator);
      }
    }

    return true;
  }

  void HandleTranslationUnit(ASTContext &context) override {
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
    clang::ASTContext &astContext = CI.getASTContext();
    for (const auto &arg : args) {
      std::string delimiter = "=";
      std::vector<std::string> components;
      size_t last = 0;
      size_t next = 0;
      while ((next = arg.find(delimiter, last)) != std::string::npos) {
        components.push_back(arg.substr(last, next - last));
        last = next + 1;
      }
      components.push_back(arg.substr(last));
      if (components[0] != "mutators") {
        clang::DiagnosticsEngine &diag = astContext.getDiagnostics();
        unsigned diagId = diag.getCustomDiagID(clang::DiagnosticsEngine::Error,
                                               "Only 'mutator=' argument is supported.");
        astContext.getDiagnostics().Report(diagId);
      }
      assert(components.size() == 2);
      mutationMap.addMutation(components.at(1));
    }
    mutationMap.setDefaultMutationsIfNotSpecified();
    return true;
  }

  // Automatically run the plugin after the main AST action
  PluginASTAction::ActionType getActionType() override {
    return AddBeforeMainAction;
  }
};

} // namespace

static FrontendPluginRegistry::Add<MullAction> X("mull-cxx-frontend", "Mull: Prepare mutations");
