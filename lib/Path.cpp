#include "mull/Path.h"

#include <llvm/Support/Path.h>
#include <llvm/Support/FileSystem.h>

std::string mull::absoluteFilePath(const std::string &directory, const std::string &filePath) {
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
