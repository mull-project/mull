#include "Filter.h"

#include <llvm/IR/DebugInfoMetadata.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Instructions.h>

using namespace llvm;
using namespace mull;

bool Filter::shouldSkipFunction(llvm::Function *function) {
  for (std::string &name : names) {
    if (function->getName().find(StringRef(name)) != StringRef::npos) {
      return true;
    }
  }

  if (function->hasMetadata()) {
    int debugInfoKindID = 0;
    MDNode *debug = function->getMetadata(debugInfoKindID);
    DISubprogram *subprogram = dyn_cast<DISubprogram>(debug);
    if (subprogram) {
      for (std::string &location : locations) {
        if (subprogram->getFilename().str().find(location) != std::string::npos) {
          return true;
        }
      }
    }
  }

  return false;
}

void Filter::skipByName(std::string &nameSubstring) {
  names.push_back(nameSubstring);
}

void Filter::skipByName(const char *nameSubstring) {
  names.push_back(std::string(nameSubstring));
}

void Filter::skipByLocation(std::string &locationSubstring) {
  locations.push_back(locationSubstring);
}

void Filter::skipByLocation(const char *locationSubstring) {
  locations.push_back(std::string(locationSubstring));
}
