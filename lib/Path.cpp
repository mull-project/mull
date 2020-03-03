#include "mull/Path.h"

#include <llvm/Support/Path.h>

std::string mull::absoluteFilePath(const std::string &directory, const std::string &filePath) {
  if (!filePath.empty() && !llvm::sys::path::is_absolute(filePath)) {
    return directory + llvm::sys::path::get_separator().str() + filePath;
  }
  return filePath;
}
