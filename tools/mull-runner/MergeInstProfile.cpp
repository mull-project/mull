#include "MergeInstProfile.h"
#include "mull/Diagnostics/Diagnostics.h"
#include <llvm/Config/llvm-config.h>
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

  // Since LLVM 14, llvm::InstrProfWriter::write returns a [[nodiscard]]-attributed
  // llvm::Error. Propagate potential errors back to the caller to avoid trigger
  // unused warnings
#if LLVM_VERSION_MAJOR >= 14
  return bool(writer.write(out));
#else
  writer.write(out);
  return true;
#endif
}
