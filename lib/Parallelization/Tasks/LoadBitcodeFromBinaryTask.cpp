#include "mull/Parallelization/Tasks/LoadBitcodeFromBinaryTask.h"

#include "mull/BitcodeLoader.h"
#include "mull/Diagnostics/Diagnostics.h"
#include "mull/Parallelization/Progress.h"

#include <ebc/EmbeddedFile.h>

#include <llvm/Support/MemoryBuffer.h>

namespace mull {

void LoadBitcodeFromBinaryTask::operator()(iterator begin, iterator end, Out &storage,
                                           mull::progress_counter &counter) {
  for (auto it = begin; it != end; it++, counter.increment()) {
    auto pair = (*it)->GetRawBuffer();
    const auto data = pair.first;
    const auto size = pair.second;
    assert(data);
    assert(size);

    llvm::StringRef bufferView(data, size);
    auto ownedBuffer = llvm::MemoryBuffer::getMemBufferCopy(bufferView);
    auto buffer = ownedBuffer.get();

    auto modulePair = mull::loadModuleFromBuffer(context, *buffer);
    auto hash = modulePair.first;
    auto module = std::move(modulePair.second);

    /// How can I check that -g flag (debug info enable) was set, from llvm pass
    /// https://stackoverflow.com/a/21713717/598057
    if (module->getNamedMetadata("llvm.dbg.cu") == nullptr) {
      diagnostics.warning("Bitcode module does not have debug information.");
    }

    assert(module && "Could not load module");
    module->setModuleIdentifier(hash);

    auto bitcode =
        llvm::make_unique<mull::Bitcode>(std::move(module), std::move(ownedBuffer), hash);
    storage.push_back(std::move(bitcode));
  }
}
} // namespace mull
