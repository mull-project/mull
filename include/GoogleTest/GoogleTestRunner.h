#pragma once

#include "TestResult.h"

#include "llvm/ExecutionEngine/Orc/ObjectLinkingLayer.h"
#include "llvm/IR/Mangler.h"
#include "llvm/Object/Binary.h"
#include "llvm/Object/ObjectFile.h"
#include "llvm/Target/TargetMachine.h"

namespace llvm {

class Function;
class Module;

}

namespace Mutang {

class GoogleTestRunner {
  llvm::orc::ObjectLinkingLayer<> ObjectLayer;
  std::unique_ptr<llvm::TargetMachine> TM;
  llvm::Mangler Mangler;
public:

  /// FIXME: these are need to be removed from here
  typedef std::vector<llvm::object::ObjectFile *> ObjectFiles;
  typedef std::vector<llvm::object::OwningBinary<llvm::object::ObjectFile>> OwnedObjectFiles;

  GoogleTestRunner();
  ExecutionResult runTest(std::vector<llvm::Function *> Ctors, llvm::Function *Test, ObjectFiles &ObjectFiles);

private:
  std::string MangleName(const llvm::StringRef &Name);
  void *TestFunctionPointer(const llvm::Function &Function);

  void runStaticCtor(llvm::Function *Ctor);
};

}
