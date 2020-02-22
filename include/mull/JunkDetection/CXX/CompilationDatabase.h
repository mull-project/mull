#pragma once

#include <map>
#include <string>
#include <utility>
#include <vector>

namespace mull {

class Diagnostics;

class CompilationDatabase {
public:
  class Path {
  public:
    explicit Path(std::string p) : path(std::move(p)) {}
    std::string getPath() const {
      return path;
    }

  private:
    std::string path;
  };
  class Flags {
  public:
    explicit Flags(std::string f) : flags(std::move(f)) {}
    std::string getFlags() const {
      return flags;
    }

  private:
    std::string flags;
  };

public:
  CompilationDatabase(Diagnostics &diagnostics, Path path, Flags flags);

  const std::vector<std::string> &compilationFlagsForFile(const std::string &filepath) const;

private:
  const std::vector<std::string> extraFlags;
  const std::map<std::string, std::vector<std::string>> database;
};

} // namespace mull