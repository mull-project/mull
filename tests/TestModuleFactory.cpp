#include "TestModuleFactory.h"

#include <llvm/AsmParser/Parser.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/Debug.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/SourceMgr.h>

#include <cassert>
#include <fstream>
#include <string>

namespace mull {

std::unique_ptr<Bitcode> loadBitcodeFromIR(const char *path) {
  ///  I cannot recall why it is implemented the way it is implemented
  ///  Sorry about that
  std::ifstream file(path);
  std::string str;
  std::string file_contents;
  while (std::getline(file, str)) {
    file_contents += str;
    file_contents.push_back('\n');
  }
  auto context = std::make_unique<llvm::LLVMContext>();
  SMDiagnostic error;
  auto module = parseAssemblyString(file_contents, error, *context);

  /// FIXME: is there another way to check for errors?
  if (!error.getMessage().empty()) {
    error.print("test", dbgs());
  }

  assert(module && "Expected module to be parsed correctly");

  assert(!llvm::verifyModule(*module, &dbgs()));

  return std::make_unique<Bitcode>(std::move(context), std::move(module));
}

} // namespace mull