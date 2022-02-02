#include "MergeInstProfile.h"
#include "mull/Diagnostics/Diagnostics.h"
#include <llvm/ProfileData/InstrProfReader.h>
#include <llvm/ProfileData/InstrProfWriter.h>
#include <llvm/Support/raw_ostream.h>

using namespace std::string_literals;

bool mull::mergeRawInstProfile(Diagnostics &diagnostics, const std::string &input,
                               const std::string &output) {
  auto maybeReader = llvm::InstrProfReader::create(input);
  if (!maybeReader) {
    diagnostics.warning("cannot read raw profile data: "s +
                        llvm::toString(maybeReader.takeError()));
    return false;
  }
  llvm::InstrProfWriter writer;
  auto &reader = maybeReader.get();
  for (auto &i : *reader) {
    writer.addRecord(std::move(i), [](llvm::Error error) {});
  }
  std::error_code ec;
  llvm::raw_fd_ostream out(output, ec);
  if (ec) {
    diagnostics.warning("cannot save indexed profile data: "s + ec.message());
    return false;
  }
  writer.write(out);
  return true;
}
