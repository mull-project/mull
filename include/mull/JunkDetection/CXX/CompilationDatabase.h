#pragma once

#include <map>
#include <string>
#include <utility>
#include <vector>

namespace mull {

class Diagnostics;

class CompilationDatabase {
public:
  using Flags = std::vector<std::string>;
  using Database = std::map<std::string, Flags>;

  CompilationDatabase() = default;
  CompilationDatabase(const CompilationDatabase &) = default;
  CompilationDatabase(CompilationDatabase &&) = default;
  CompilationDatabase &operator=(const CompilationDatabase &) = default;
  CompilationDatabase &operator=(CompilationDatabase &&) = default;

  CompilationDatabase(Database database, Flags extraFlags, Database bitcodeFlags);

  static CompilationDatabase fromFile(Diagnostics &diagnostics, const std::string &path,
                                      const std::string& extraFlags,
                                      const std::map<std::string, std::string>& bitcodeFlags);
  static CompilationDatabase fromBuffer(Diagnostics &diagnostics, const std::string &buffer,
                                        const std::string& extraFlags,
                                        const std::map<std::string, std::string>& bitcodeFlags);

  const CompilationDatabase::Flags &compilationFlagsForFile(const std::string &filepath) const;

private:
  Flags extraFlags;
  Database database;
  Database bitcodeFlags;
};

} // namespace mull
