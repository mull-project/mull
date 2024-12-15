#pragma once

#include <memory>
#include <tuple>

namespace clang {
class ASTUnit;
}

namespace llvm {
class LLVMContext;
class MemoryBuffer;
class Module;
} // namespace llvm

namespace mull_test {

class InMemoryFixture {
public:
  /// Warning: Clang API used by createAST: clang::tooling::buildASTFromCode
  /// seems to not make a copy of the source code string buffer provided to it
  /// so we only allow const std::string references and prevent implicit
  /// conversions from char * and const char *arguments.
  /// Otherwise, a very buggy behavior could be caused by char * string to be
  /// implicitly copied to a const string with a reference which would then
  /// result in a memory corruption because the temporary std::string object
  /// would get deleted.
  std::unique_ptr<clang::ASTUnit> createAST(const std::string &sourceCode);
  std::unique_ptr<clang::ASTUnit> createAST(char *sourceCode) = delete;
  std::unique_ptr<clang::ASTUnit> createAST(const char *sourceCode) = delete;

  std::unique_ptr<llvm::Module> createIR(const std::string &sourceCode, llvm::LLVMContext &context);
  std::unique_ptr<llvm::MemoryBuffer> createMemoryBuffer(const std::string &sourceCode);
};

} // namespace mull_test
