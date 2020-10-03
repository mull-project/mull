#include "mull/Version.h"

#include <llvm/Support/raw_ostream.h>

namespace mull {

const char *mullVersionString() { return "@PROJECT_VERSION@"; }
const char *mullCommitString() { return "@GIT_COMMIT@"; }
const char *mullBuildDateString() { return "@BUILD_DATE@"; }
const char *mullDescriptionString() { return "@PROJECT_DESCRIPTION@"; }
const char *mullHomepageString() { return "@PROJECT_HOMEPAGE_URL@"; }

const char *llvmVersionString() { return "@LLVM_VERSION@"; }

void printVersionInformation(llvm::raw_ostream &out) {
  out << "Mull: " << mullDescriptionString() << "\n";
  out << mullHomepageString() << "\n";
  out << "Version: " << mullVersionString() << "\n";
  out << "Commit: " << mullCommitString() << "\n";
  out << "Date: " << mullBuildDateString() << "\n";
  out << "LLVM: " << llvmVersionString() << "\n";
}

} // namespace mull