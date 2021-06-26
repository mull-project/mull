#include "InMemoryCompiler.h"

#include <clang/Basic/DiagnosticOptions.h>
#include <clang/Basic/TargetInfo.h>
#include <clang/CodeGen/CodeGenAction.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/TextDiagnosticPrinter.h>
#include <clang/Lex/PreprocessorOptions.h>

#include <llvm/IR/Module.h>
#include <llvm/Support/TargetSelect.h>
#if LLVM_VERSION_MAJOR >= 11
#include <llvm/Support/Host.h>
#endif

using namespace mull_test;

std::unique_ptr<llvm::Module> InMemoryCompiler::compile(const std::string &code,
                                                        const std::string &inMemoryFileName,
                                                        llvm::LLVMContext &context) {
  /// The following code is based on the following sources:
  /// 1. http://blog.audio-tk.com/2018/09/18/compiling-c-code-in-memory-with-clang/
  /// 2.
  /// https://stackoverflow.com/questions/34828480/generate-assembly-from-c-code-in-memory-using-libclang/34866053#34866053

  llvm::InitializeNativeTarget();
  llvm::InitializeNativeTargetAsmPrinter();
  llvm::InitializeNativeTargetAsmParser();

  /// Prepare compilation arguments
  std::vector<const char *> args;
  args.push_back(inMemoryFileName.c_str());

  /// Create and initialize CompilerInstance
  clang::CompilerInstance clangCompilerInstance;
  clangCompilerInstance.createDiagnostics();

  /// Initialize CompilerInvocation
  clang::CompilerInvocation &compilerInvocation = clangCompilerInstance.getInvocation();
#if LLVM_VERSION_MAJOR > 9
  clang::CompilerInvocation::CreateFromArgs(
      compilerInvocation, args, clangCompilerInstance.getDiagnostics());
#else
  clang::CompilerInvocation::CreateFromArgs(
      compilerInvocation, &args[0], &args[0] + args.size(), clangCompilerInstance.getDiagnostics());
#endif
  /// Configure options

  const auto languageOptions = compilerInvocation.getLangOpts();
  languageOptions->CPlusPlus = 1;
  languageOptions->CPlusPlus11 = 1;
  languageOptions->Bool = 1;

  /// auto& preprocessorOptions = compilerInvocation->getPreprocessorOpts();
  /// auto& targetOptions = compilerInvocation->getTargetOpts();
  auto &frontEndOptions = compilerInvocation.getFrontendOpts();
  frontEndOptions.ShowStats = false;

  auto &headerSearchOptions = compilerInvocation.getHeaderSearchOpts();
  headerSearchOptions.Verbose = false;
  auto &codeGenOptions = compilerInvocation.getCodeGenOpts();
  codeGenOptions.setDebugInfo(clang::codegenoptions::FullDebugInfo);
  codeGenOptions.DebugColumnInfo = 1;
  codeGenOptions.DebugCompilationDir = "/";
  codeGenOptions.MainFileName = "/in-memory-file.cc";

  /// Map code filename to a memoryBuffer
  llvm::StringRef testCodeData(code);
  std::unique_ptr<llvm::MemoryBuffer> buffer = llvm::MemoryBuffer::getMemBufferCopy(testCodeData);
  compilerInvocation.getPreprocessorOpts().addRemappedFile(inMemoryFileName, buffer.get());

  /// Configure target options
  const std::shared_ptr<clang::TargetOptions> targetOptions =
      std::make_shared<clang::TargetOptions>();
  targetOptions->Triple = llvm::sys::getDefaultTargetTriple();
  clang::TargetInfo *pTargetInfo =
      clang::TargetInfo::CreateTargetInfo(clangCompilerInstance.getDiagnostics(), targetOptions);
  clangCompilerInstance.setTarget(pTargetInfo);

  /// Create and execute action
  std::unique_ptr<clang::CodeGenAction> compilerAction(new clang::EmitLLVMOnlyAction(&context));
  bool actionResult = clangCompilerInstance.ExecuteAction(*compilerAction);
  (void)actionResult;
  assert(actionResult);

  /// Obtain LLVM Module
  std::unique_ptr<llvm::Module> module = compilerAction->takeModule();
  assert(module);

  buffer.release();

  return module;
}
