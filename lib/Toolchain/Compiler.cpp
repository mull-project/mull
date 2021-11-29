#include "mull/Toolchain/Compiler.h"

#include "LLVMCompatibility.h"
#include "mull/Bitcode.h"
#include "mull/Config/Configuration.h"
#include "mull/Diagnostics/Diagnostics.h"

#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/DynamicLibrary.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/TargetRegistry.h>
#include <llvm/Target/TargetOptions.h>

using namespace llvm;
using namespace llvm::object;
using namespace mull;
using namespace std::string_literals;

Compiler::Compiler(Diagnostics &diagnostics, const Configuration &configuration)
    : diagnostics(diagnostics), configuration(configuration) {}

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

  std::string result = tempFile(diagnostics, configuration.lowerBitcode ? "o" : "bc");
  std::error_code errorCode;
  llvm::raw_fd_ostream dest(result,
                            errorCode,
#if LLVM_VERSION_MAJOR >= 13
                            llvm::sys::fs::OpenFlags::OF_None
#else
                            llvm::sys::fs::OpenFlags::F_None
#endif
  );

  if (errorCode) {
    diagnostics.error("Could not open file: "s + errorCode.message());
    return std::string();
  }

  if (!configuration.lowerBitcode) {
    llvm_compat::writeBitcodeToFile(*bitcode.getModule(), dest);
    return result;
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
  llvm::Optional<llvm::Reloc::Model> relocationModel(llvm::Reloc::Model::PIC_);
  llvm::TargetMachine *targetMachine =
      target->createTargetMachine(targetTriple, CPU, features, opt, relocationModel);

  llvm::legacy::PassManager pass;
  if (llvm_compat::addPassesToEmitObjectFile(targetMachine, pass, dest)) {
    diagnostics.error("TargetMachine can not emit object file");
    return std::string();
  }
  pass.run(*bitcode.getModule());
  dest.flush();

  if (configuration.debugEnabled) {
    std::string bitcodePath = tempFile(diagnostics, "bc");
    llvm::raw_fd_ostream bcStream(bitcodePath,
                                  errorCode,
#if LLVM_VERSION_MAJOR >= 13
                                  llvm::sys::fs::OpenFlags::OF_None
#else
                                  llvm::sys::fs::OpenFlags::F_None
#endif
    );
    if (errorCode) {
      diagnostics.warning("Could not open temp bc file: "s + errorCode.message());
      return std::string();
    }
    llvm_compat::writeBitcodeToFile(*bitcode.getModule(), bcStream);
    diagnostics.debug("Emitted object file: "s + result + " for bitcode file: "s + bitcodePath);
  }
  return result;
}
