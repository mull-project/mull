#include "Compiler.h"
#include "ModuleLoader.h"

#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/Orc/CompileUtils.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/TargetSelect.h"

#include "llvm/IR/DataLayout.h"

using namespace llvm;
using namespace llvm::object;
using namespace Mutang;

static void putFileIntoCache(ObjectFile *object, std::string uniqueID) {
  std::string cacheName("/tmp/mutang_cache/" + uniqueID + ".o");
  std::error_code EC;
  raw_fd_ostream outfile(cacheName, EC, sys::fs::F_None);
  outfile.write(object->getMemoryBufferRef().getBufferStart(),
                object->getMemoryBufferRef().getBufferSize());
  outfile.close();
}

static OwningBinary<ObjectFile> loadFileFromCache(std::string uniqueID) {
  std::string cacheName("/tmp/mutang_cache/" + uniqueID + ".o");

  ErrorOr<std::unique_ptr<MemoryBuffer>> buffer =
    MemoryBuffer::getFile(cacheName.c_str());

  if (!buffer) {
    return OwningBinary<ObjectFile>();
  }

  Expected<std::unique_ptr<ObjectFile>> object =
    ObjectFile::createObjectFile(buffer.get()->getMemBufferRef());

  if (!object) {
    return OwningBinary<ObjectFile>();
  }

  return OwningBinary<ObjectFile>(std::move(*object),
                                  std::move(buffer.get()));
}

Compiler::Compiler(bool cache) : useCache(cache) {
  InitializeNativeTarget();
  InitializeNativeTargetAsmPrinter();
  InitializeNativeTargetAsmParser();
}

OwningBinary<ObjectFile> Compiler::compileModule(const MutangModule &module) {
  /// FIXME: Initialize everything once
  std::unique_ptr<TargetMachine> targetMachine(
                                 EngineBuilder().selectTarget(Triple(), "", "",
                                              SmallVector<std::string, 1>()));
  assert(targetMachine && "Can't create TargetMachine");

  Module *llvmModule = module.getModule();

  if (llvmModule->getDataLayout().isDefault()) {
    llvmModule->setDataLayout(targetMachine->createDataLayout());
  }

  orc::SimpleCompiler compiler(*targetMachine);

  OwningBinary<ObjectFile> objectFile = compiler(*llvmModule);
  
  return objectFile;
}

OwningBinary<ObjectFile> Compiler::compileModule(MutangModule *module,
                                                 const std::string &uniqueID) {
  return compileModule(module->getModule(), uniqueID);
}

OwningBinary<ObjectFile> Compiler::compileModule(Module *module,
                                                 const std::string &uniqueID) {
  assert(module);

  if (useCache) {
    OwningBinary<ObjectFile> objectFile = loadFileFromCache(uniqueID);
    if (objectFile.getBinary() != nullptr) {
      printf("Cache-hit %s\n", uniqueID.c_str());
      return objectFile;
    }
  }

  /*
   
  OwningBinary<ObjectFile> objectFile;

  if (useCache) {
    objectFile = loadFileFromCache(uniqueID);
    if (objectFile.getBinary() == nullptr) {
      objectFile = compileModule(module->getModule(), uniqueID);
      putFileIntoCache(objectFile.getBinary(), uniqueID);
    } else {
      printf("Cache-hit %s\n", uniqueID.c_str());
    }
  } else {
    objectFile = compileModule(module->getModule(), uniqueID);
  }

  return objectFile;

   */

  printf("Compiling %s\n", uniqueID.c_str());

  /// FIXME: Initialize everything once
  std::unique_ptr<TargetMachine> targetMachine(
                              EngineBuilder().selectTarget(Triple(), "", "",
                              SmallVector<std::string, 1>()));

  assert(targetMachine && "Can't create TargetMachine");

  if (module->getDataLayout().isDefault()) {
    module->setDataLayout(targetMachine->createDataLayout());
  }

  orc::SimpleCompiler compiler(*targetMachine);

  OwningBinary<ObjectFile> objectFile = compiler(*module);

  if (useCache) {
    putFileIntoCache(objectFile.getBinary(), uniqueID);
  }

  return objectFile;
}
