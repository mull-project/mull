#include "mull/Toolchain/Compiler.h"

#include "LLVMCompatibility.h"
#include "mull/Bitcode.h"
#include "mull/Config/Configuration.h"
#include "mull/Diagnostics/Diagnostics.h"

#include <llvm/ExecutionEngine/Orc/CompileUtils.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/DynamicLibrary.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/TargetRegistry.h>

using namespace llvm;
using namespace llvm::object;
using namespace mull;
using namespace std::string_literals;

Compiler::Compiler(Diagnostics &diagnostics, const Configuration &configuration)
    : diagnostics(diagnostics), configuration(configuration) {}

OwningBinary<ObjectFile> Compiler::compileBitcode(const Bitcode &bitcode, TargetMachine &machine) {
  return compileModule(bitcode.getModule(), machine);
}

OwningBinary<ObjectFile> Compiler::compileModule(Module *module, TargetMachine &machine) {
  assert(module);

  std::string error;
  llvm::raw_string_ostream stream(error);
  if (llvm::verifyModule(*module, &stream)) {
    stream.flush();
    diagnostics.error(error);
  }

  if (module->getDataLayout().isDefault()) {
    module->setDataLayout(machine.createDataLayout());
  }

  orc::SimpleCompiler compiler(machine);

  OwningBinary<ObjectFile> objectFile = llvm_compat::compileModule(compiler, *module);

  return objectFile;
}

static std::string tempFile(Diagnostics &diagnostics, const std::string &extension) {
  llvm::Twine prefix("mull");
  llvm::SmallString<128> resultPath;
  if (std::error_code err = llvm::sys::fs::createTemporaryFile(prefix, extension, resultPath)) {
    diagnostics.error("Cannot create temporary file"s + err.message());
    return std::string();
  }
  return resultPath.str().str();
}

std::string Compiler::compileBitcode(const Bitcode &bitcode) {
  std::string error;
  llvm::raw_string_ostream stream(error);
  if (llvm::verifyModule(*bitcode.getModule(), &stream)) {
    stream.flush();
    diagnostics.error(error);
  }

  const std::string &targetTriple = bitcode.getModule()->getTargetTriple();
  const llvm::Target *target = llvm::TargetRegistry::lookupTarget(targetTriple, error);
  if (!target) {
    diagnostics.error("Cannot lookup target: "s + error + '\n');
    return std::string();
  }

  auto CPU = "generic";
  auto features = "";
  llvm::TargetOptions opt;
  llvm::Optional<llvm::Reloc::Model> relocationModel;
  llvm::TargetMachine *targetMachine =
      target->createTargetMachine(targetTriple, CPU, features, opt, relocationModel);

  std::string result = tempFile(diagnostics, "o");
  std::error_code errorCode;
  llvm::raw_fd_ostream dest(result, errorCode, llvm::sys::fs::OpenFlags::F_None);

  if (errorCode) {
    diagnostics.error("Could not open file: "s + errorCode.message());
    return std::string();
  }

  llvm::legacy::PassManager pass;
  if (llvm_compat::addPassesToEmitObjectFile(targetMachine, pass, dest)) {
    diagnostics.error("TargetMachine can not emit object file");
    return std::string();
  }
  pass.run(*bitcode.getModule());
  dest.flush();

  if (configuration.debugEnabled) {
    std::string bitcodePath = tempFile(diagnostics, "bc");
    llvm::raw_fd_ostream bcStream(bitcodePath, errorCode, llvm::sys::fs::OpenFlags::F_None);
    if (errorCode) {
      diagnostics.warning("Could not open temp bc file: "s + errorCode.message());
      return std::string();
    }
    llvm_compat::writeBitcodeToFile(*bitcode.getModule(), bcStream);
    //    llvm_compat::writeBitcodeToFile(*bitcode.getModule(), dest);
    diagnostics.debug("Emitted object file: "s + result + " for bitcode file: "s + bitcodePath);
  }
  return result;
}
