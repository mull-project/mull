#pragma once

namespace llvm {
class raw_ostream;
}

namespace mull {

const char *mullVersionString();
const char *mullCommitString();
const char *mullDescriptionString();
const char *mullHomepageString();
const char *mullDocsPageString();
const char *mullBuildDateString();

const char *llvmVersionString();

void printVersionInformation(llvm::raw_ostream &out);

} // namespace mull
