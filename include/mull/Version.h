#pragma once

namespace llvm {
class raw_ostream;
}

namespace mull {

const char *mullVersionString();
const char *mullCommitString();
const char *mullDescriptionString();
const char *mullHomepageString();
const char *mullBuildDateString();

const char *llvmVersionString();

void printVersionInformation();
void printVersionInformationStream(llvm::raw_ostream &out);

} // namespace mull
