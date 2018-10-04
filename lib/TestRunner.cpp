#include "TestRunner.h"

#include <llvm/Support/DynamicLibrary.h>

mull::TestRunner::TestRunner()
{
  llvm::sys::DynamicLibrary::LoadLibraryPermanently(nullptr);
}

