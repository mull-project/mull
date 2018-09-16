#include "Parallelization/Tasks/LoadObjectFilesTask.h"
#include "Parallelization/Progress.h"

#include "Logger.h"
#include "LLVMCompatibility.h"

using namespace mull;
using namespace llvm;

void mull::LoadObjectFilesTask::operator()(mull::LoadObjectFilesTask::iterator begin,
                                           mull::LoadObjectFilesTask::iterator end,
                                           mull::LoadObjectFilesTask::Out &storage,
                                           mull::progress_counter &counter) {
  for (auto it = begin; it != end; it++, counter.increment()) {
    auto objectFilePath = *it;
    ErrorOr<std::unique_ptr<MemoryBuffer>> buffer =
        MemoryBuffer::getFile(objectFilePath);

    if (!buffer) {
      Logger::error() << "Cannot load object file: " << objectFilePath << "\n";
      continue;
    }

    llvm_compat::Expected<std::unique_ptr<object::ObjectFile>> objectOrError =
        object::ObjectFile::createObjectFile(buffer.get()->getMemBufferRef());

    if (!objectOrError) {
      Logger::error() << "Cannot create object file: " << objectFilePath << "\n";
      continue;
    }

    std::unique_ptr<object::ObjectFile> objectFile(std::move(objectOrError.get()));

    auto owningObject = object::OwningBinary<object::ObjectFile>(std::move(objectFile),
                                                                 std::move(buffer.get()));
    storage.push_back(std::move(owningObject));
  }
}
