#include "ASTInstrumentation.h"
#include "ASTMutation.h"
#include "ASTMutationsSearchVisitor.h"
#include "ASTNodeFactory.h"
#include "MullASTMutator.h"

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
  std::unordered_set<mull::MutatorKind> usedMutatorSet;

public:
  MullASTConsumer(CompilerInstance &Instance, std::unordered_set<mull::MutatorKind> usedMutatorSet)
      : Instance(Instance), astMutator(nullptr), usedMutatorSet(usedMutatorSet) {}

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

      ASTMutationsSearchVisitor visitor(Instance.getASTContext(), usedMutatorSet);
      errs() << "HandleTopLevelDecl: Looking at function: " << f->getDeclName() << "\n";
      visitor.TraverseFunctionDecl(f);
      std::vector<ASTMutation> foundMutations = visitor.getAstMutations();
      astMutator->performMutations(foundMutations);
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
  std::unordered_set<mull::MutatorKind> usedMutatorSet;

protected:
  std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI, llvm::StringRef) override {
    return std::make_unique<MullASTConsumer>(CI, usedMutatorSet);
  }

  bool ParseArgs(const CompilerInstance &CI, const std::vector<std::string> &args) override {
    const std::unordered_map<std::string, mull::MutatorKind> argsToMutatorsMap = {
      { "cxx_add_to_sub", mull::MutatorKind::CXX_AddToSub },
      { "cxx_sub_to_add", mull::MutatorKind::CXX_SubToAdd },
      { "cxx_logical_or_to_and", mull::MutatorKind::CXX_Logical_OrToAnd },
      { "cxx_remove_void_call", mull::MutatorKind::CXX_RemoveVoidCall },
    };
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
      assert(argsToMutatorsMap.count(components.at(1)) != 0);
      usedMutatorSet.insert(argsToMutatorsMap.at(components[1]));
    }
    if (usedMutatorSet.empty()) {
      usedMutatorSet.insert(mull::MutatorKind::CXX_AddToSub);
      usedMutatorSet.insert(mull::MutatorKind::CXX_Logical_OrToAnd);
      usedMutatorSet.insert(mull::MutatorKind::CXX_RemoveVoidCall);
    }
    return true;
  }

  // Automatically run the plugin after the main AST action
  PluginASTAction::ActionType getActionType() override {
    return AddBeforeMainAction;
  }
};

} // namespace

static FrontendPluginRegistry::Add<MullAction> X("mull-cxx-frontend", "Mull: Prepare mutations");
