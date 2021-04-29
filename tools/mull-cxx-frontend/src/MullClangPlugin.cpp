#include "ASTMutation.h"
#include "ASTMutationsSearchVisitor.h"
#include "ASTMutator.h"
#include "MullTreeTransform.h"
#include "ReadOnlyVisitor.h"

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
  FunctionDecl *getenvFuncDecl;
  std::unique_ptr<MullTreeTransform> treeTransform;
  std::unique_ptr<ASTMutator> astMutator;

public:
  MullASTConsumer(CompilerInstance &Instance)
      : Instance(Instance), getenvFuncDecl(nullptr), treeTransform(nullptr), astMutator(nullptr) {}

  void Initialize(ASTContext &Context) override {
    ASTConsumer::Initialize(Context);

    /// Create an external getenv() declaration.
    LinkageSpecDecl *cLinkageSpecDecl =
        LinkageSpecDecl::Create(Context,
                                Context.getTranslationUnitDecl(),
                                SourceLocation(),
                                SourceLocation(),
                                LinkageSpecDecl::LanguageIDs::lang_c,
                                true);
    this->getenvFuncDecl = createGetEnvFuncDecl(Context, cLinkageSpecDecl);
    cLinkageSpecDecl->addDecl(this->getenvFuncDecl);
    Context.getTranslationUnitDecl()->addDecl(cLinkageSpecDecl);
  }

  bool HandleTopLevelDecl(DeclGroupRef DG) override {
    llvm::errs() << "BEGIN: HandleTopLevelDecl():\n";

    /// Should be a better place to create this. But at Initialize(), getSema() hits an internal
    /// assert.
    if (!treeTransform && !astMutator) {
      treeTransform = std::make_unique<MullTreeTransform>(Instance.getSema());
      astMutator = std::make_unique<ASTMutator>(Instance.getASTContext(), getenvFuncDecl);
    }

    ASTMutationsSearchVisitor visitor;

    Instance.getASTContext().getDiagnostics();
    auto translationUnitDecl = Instance.getASTContext().getTranslationUnitDecl();
    if (!translationUnitDecl) {
      printf("translationUnitDecl is nullptr\n");
      exit(1);
    }

    for (DeclGroupRef::iterator I = DG.begin(), E = DG.end(); I != E; ++I) {
      if ((*I)->getKind() != Decl::Function) {
        continue;
      }
      FunctionDecl *f = static_cast<FunctionDecl *>(*I);
      DeclarationName origName(f->getDeclName());
      if (origName.getAsString() == "main") {
        continue;
      }

      errs() << "Looking at function: " << f->getName() << "\n";
      visitor.TraverseFunctionDecl(f);
    }

    std::vector<ASTMutation> foundMutations = visitor.getAstMutations();
    performMutations(foundMutations);

    errs() << "FINISHED: HandleTopLevelDecl\n";
    return true;
  }

  void HandleTranslationUnit(ASTContext &context) override {
    llvm::errs() << "BEGIN: HandleTranslationUnit():\n";

    ReadOnlyVisitor readOnlyVisitor;
    readOnlyVisitor.TraverseTranslationUnitDecl(Instance.getASTContext().getTranslationUnitDecl());

    errs() << "FINISHED: HandleTranslationUnit\n";
  }

  clang::FunctionDecl *createGetEnvFuncDecl(ASTContext &context, clang::DeclContext *declContext) {
    /// FunctionDecl 0x7fc7b8897ef0
    /// </Users/Stanislaw/workspace/projects/poc-sandbox/clang-plugin-test/src/fixture.cpp:3:1,
    /// col:33> col:14 getenv 'char *(const char *)' extern
    /// `-ParmVarDecl 0x7fc7b8897e28 <col:21, col:32> col:33 'const char *'

    clang::IdentifierInfo &getenvFuncIdentifierInfo = context.Idents.get("getenv");
    clang::IdentifierInfo &getenvParamNameInfo = context.Idents.get("name");
    clang::DeclarationName declarationName(&getenvFuncIdentifierInfo);

    clang::FunctionProtoType::ExtProtoInfo ext;

    clang::QualType parameterType = context.getPointerType(context.getConstType(context.CharTy));
    clang::QualType returnType = context.getPointerType(context.CharTy);

    std::vector<clang::QualType> paramTypes;
    paramTypes.push_back(parameterType);

    clang::QualType getenvFuncType = context.getFunctionType(returnType, paramTypes, ext);

    clang::TypeSourceInfo *trivialSourceInfo = context.getTrivialTypeSourceInfo(getenvFuncType);

    clang::FunctionDecl *getEnvFuncDecl = clang::FunctionDecl::Create(
        context,
        declContext,
        SourceLocation(),
        SourceLocation(),
        declarationName,
        getenvFuncType,
        trivialSourceInfo,
        clang::StorageClass::SC_Extern,
        false,          /// bool isInlineSpecified = false,
        true,           /// bool hasWrittenPrototype = true,
        CSK_unspecified /// ConstexprSpecKind ConstexprKind = CSK_unspecified
    );

    clang::ParmVarDecl *ParDecl = ParmVarDecl::Create(context,
                                                      getEnvFuncDecl,
                                                      SourceLocation(),
                                                      SourceLocation(),
                                                      &getenvParamNameInfo,
                                                      parameterType,
                                                      trivialSourceInfo,
                                                      StorageClass::SC_None,
                                                      nullptr // Expr *DefArg
    );

    std::vector<clang::ParmVarDecl *> paramDecls;
    paramDecls.push_back(ParDecl);
    getEnvFuncDecl->setParams(paramDecls);

    return getEnvFuncDecl;
  }

  void performMutations(std::vector<ASTMutation> &astMutations) {
    for (ASTMutation &astMutation : astMutations) {
      errs() << "Before Perform MUTATION:"
             << "\n";

      clang::BinaryOperator *oldBinaryOperator =
          dyn_cast<clang::BinaryOperator>(astMutation.mutableStmt);
      ExprResult exprResult = treeTransform->TransformBinaryOperator(oldBinaryOperator);
      clang::BinaryOperator *newBinaryOperator = (clang::BinaryOperator *)exprResult.get();

      if (astMutation.mutationType == mull::MutatorKind::CXX_AddToSub) {
        newBinaryOperator->setOpcode(BinaryOperator::Opcode::BO_Sub);
      } else if (astMutation.mutationType == mull::MutatorKind::CXX_Logical_OrToAnd) {
        newBinaryOperator->setOpcode(BinaryOperator::Opcode::BO_LAnd);
      } else {
        assert(0 && "Not implemented");
      }

      astMutator->replaceStatement(oldBinaryOperator, newBinaryOperator);

      errs() << "After Perform MUTATION:"
             << "\n";
    }
  }
};

class MullAction : public PluginASTAction {
protected:
  std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI, llvm::StringRef) override {
    return llvm::make_unique<MullASTConsumer>(CI);
  }

  bool ParseArgs(const CompilerInstance &CI, const std::vector<std::string> &args) override {
    return true;
  }

  // Automatically run the plugin after the main AST action
  PluginASTAction::ActionType getActionType() override {
    return AddBeforeMainAction;
  }
};

} // namespace

static FrontendPluginRegistry::Add<MullAction> X("mull-cxx-frontend", "Mull: Prepare mutations");
