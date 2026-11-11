#include "mull/Path.h"

#include <limits.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Path.h>
#include <mutex>
#include <shared_mutex>
#include <unordered_map>

static std::string resolveFilePath(const std::string &directory, const std::string &filePath) {
  auto path = filePath;
  if (!filePath.empty() && !llvm::sys::path::is_absolute(filePath)) {
    path = directory + llvm::sys::path::get_separator().str() + filePath;
  }
  llvm::SmallString<PATH_MAX> realPath;
  if (!llvm::sys::fs::real_path(path, realPath, true)) {
    return realPath.str().str();
  }
  return path;
}

std::string mull::absoluteFilePath(const std::string &directory, const std::string &filePath) {
  /// Called per instruction and mutation point, real_path is a syscall per path component
  static std::shared_mutex mutex;
  static std::unordered_map<std::string, std::string> cache;
  std::string key = directory + '\0' + filePath;
  {
    std::shared_lock lock(mutex);
    auto it = cache.find(key);
    if (it != cache.end()) {
      return it->second;
    }
  }
  std::string path = resolveFilePath(directory, filePath);
  std::unique_lock lock(mutex);
  return cache.emplace(std::move(key), std::move(path)).first->second;
}
