#include "mull/Parallelization/Tasks/LoadObjectFilesTask.h"

#include "mull/Logger.h"
#include "mull/Parallelization/Progress.h"

using namespace mull;
using namespace llvm;

void LoadObjectFilesTask::operator()(iterator begin, iterator end, Out &storage,
                                     progress_counter &counter) {
  for (auto it = begin; it != end; it++, counter.increment()) {
    auto objectFilePath = *it;
    ErrorOr<std::unique_ptr<MemoryBuffer>> buffer =
        MemoryBuffer::getFile(objectFilePath);

    if (!buffer) {
      Logger::error() << "Cannot load object file: " << objectFilePath << "\n";
      continue;
    }

    Expected<std::unique_ptr<object::ObjectFile>> objectOrError =
        object::ObjectFile::createObjectFile(buffer.get()->getMemBufferRef());

    if (!objectOrError) {
      Logger::error() << "Cannot create object file: " << objectFilePath
                      << "\n";
      continue;
    }

    std::unique_ptr<object::ObjectFile> objectFile(
        std::move(objectOrError.get()));

    auto owningObject = object::OwningBinary<object::ObjectFile>(
        std::move(objectFile), std::move(buffer.get()));
    storage.push_back(std::move(owningObject));
  }
}
