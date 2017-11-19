#include "Mangler.h"

#include "llvm/ADT/Twine.h"
#include <llvm/IR/DataLayout.h>
#include <llvm/IR/Mangler.h>
#include <llvm/Support/raw_ostream.h>

namespace mull {

std::string Mangler::getNameWithPrefix(const std::string &name,
                                       const llvm::DataLayout &dataLayout) {
  const llvm::StringRef stringRefName = name;
  std::string mangledName;

  llvm::raw_string_ostream stream(mangledName);
  llvm::Mangler::getNameWithPrefix(stream,
                                   stringRefName,
                                   dataLayout);

  return mangledName;
}

}
