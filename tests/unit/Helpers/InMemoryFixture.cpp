#include "InMemoryFixture.h"

#include "Helpers/InMemoryCompiler.h"

#include <cassert>
#include <clang/Frontend/ASTUnit.h>
#include <clang/Tooling/Tooling.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>

using namespace mull_test;

/// When unit testing, we compile code from C++ string literals with
/// mull_test::InMemoryCompiler. In that case the special convention for the
/// code compiled from memory is that the file name is "/in-memory-file.cc".
/// We don't want to check for presence of such in-memory files on a real file
/// system.
static const char *const fakeSourceFilePath = "/in-memory-file.cc";

std::unique_ptr<clang::ASTUnit> InMemoryFixture::createAST(const std::string &sourceCode) {
  static const std::vector<std::string> args = {
    "-std=c++11"
  };

  std::unique_ptr<clang::ASTUnit> astUnit =
      clang::tooling::buildASTFromCodeWithArgs(sourceCode, args, fakeSourceFilePath);
  assert(astUnit);
  return astUnit;
}

std::unique_ptr<llvm::Module> InMemoryFixture::createIR(const std::string &sourceCode,
                                                        llvm::LLVMContext &context) {
  mull_test::InMemoryCompiler compiler;
  std::unique_ptr<llvm::Module> module = compiler.compile(sourceCode, fakeSourceFilePath, context);
  assert(module);
  return module;
}

std::unique_ptr<llvm::MemoryBuffer>
InMemoryFixture::createMemoryBuffer(const std::string &sourceCode) {
  std::unique_ptr<llvm::MemoryBuffer> sourceCodeAsBuffer =
      llvm::MemoryBuffer::getMemBufferCopy(sourceCode);
  assert(sourceCodeAsBuffer->getBufferSize() > 0);
  return sourceCodeAsBuffer;
}
