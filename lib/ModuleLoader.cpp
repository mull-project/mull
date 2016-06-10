#include "ModuleLoader.h"

#include "llvm/AsmParser/Parser.h"
#include "llvm/Bitcode/ReaderWriter.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/SourceMgr.h"

using namespace llvm;
using namespace Mutang;

std::unique_ptr<Module> ModuleLoader::loadModuleAtPath(const std::string &path) {
  auto BufferOrError = MemoryBuffer::getFile(path);
  if (!BufferOrError) {
    printf("can't load module '%s'\n", path.c_str());
    return nullptr;
  }

  auto Module = parseBitcodeFile(BufferOrError->get()->getMemBufferRef(), Ctx);
  if (!Module) {
    printf("can't load module '%s'\n", path.c_str());
    return nullptr;
  }

  return std::move(Module.get());
}
