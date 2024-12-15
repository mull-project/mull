#include "FixturePaths.h"
#include "mull/JunkDetection/CXX/CompilationDatabase.h"
#include <gtest/gtest.h>
#include <mull/Diagnostics/Diagnostics.h>

#include <string>

using namespace mull;
using namespace std::string_literals;

TEST(CompilationDatabaseFromCompilationFlags, returnsAllFlags) {
  Diagnostics diagnostics;
  const CompilationDatabase database =
      CompilationDatabase::fromBuffer(diagnostics, "", "-I     /usr/foo/include   ", {});

  const std::string filepath("foobar");
  const auto &[_, flags] = database.compilationFlagsForFile(filepath);
  ASSERT_EQ(flags.size(), size_t(2));
}

TEST(CompilationDatabaseFromCompilationFlags, returnsAllFlagsForAnyFile) {
  Diagnostics diagnostics;
  const CompilationDatabase database =
      CompilationDatabase::fromBuffer(diagnostics, "", "-I /usr/foo/include   ", {});

  const std::string foo("foo");
  const std::string bar("bar");

  const auto &[_0, fooFlags] = database.compilationFlagsForFile(foo);
  const auto &[_1, barFlags] = database.compilationFlagsForFile(bar);

  ASSERT_EQ(fooFlags.size(), barFlags.size());

  for (size_t index(0); index < fooFlags.size(); index++) {
    auto &fooFlag = fooFlags.at(index);
    auto &barFlag = barFlags.at(index);

    ASSERT_EQ(fooFlag, barFlag);
  }
}

TEST(CompilationDatabaseFromFile, loadsFromValidFiles) {
  Diagnostics diagnostics;
  const std::vector<std::string> databasePaths({
      fixtures::junk_detection_compdb_db_with_arguments_json_path(),
      fixtures::junk_detection_compdb_db_with_commands_json_path(),
      fixtures::junk_detection_compdb_db_with_output_json_path(),
  });

  std::vector<std::string> files(
      { "/foo/bar/foobar.cpp", "/foo/bar/./foobar.cpp", "/foo/bar/buzz/../foobar.cpp" });
  for (const std::string &file : files) {
    for (auto &path : databasePaths) {
      const CompilationDatabase database = CompilationDatabase::fromFile(diagnostics, path, "", {});

      auto &[_, compilationFlags] = database.compilationFlagsForFile(file);
      ASSERT_EQ(compilationFlags.size(), size_t(6));

      ASSERT_EQ(compilationFlags.at(0), "-I"s);
      ASSERT_EQ(compilationFlags.at(1), "foo"s);
      ASSERT_EQ(compilationFlags.at(2), "-I"s);
      ASSERT_EQ(compilationFlags.at(3), "bar"s);
      ASSERT_EQ(compilationFlags.at(4), "-c"s);
      ASSERT_EQ(compilationFlags.at(5), "foobar.cpp"s);
    }
  }
}

TEST(CompilationDatabaseFromFile, includesCompilationFlagsPassedSeparately) {
  Diagnostics diagnostics;
  auto path = fixtures::junk_detection_compdb_db_with_fullpath_compiler_json_path();
  const CompilationDatabase database = CompilationDatabase::fromFile(
      diagnostics, path, "-isystem /usr/local/include -isystem /usr/include", {});

  const std::string file("/foo/bar/foobar.cpp");
  auto &[_, compilationFlags] = database.compilationFlagsForFile(file);
  ASSERT_EQ(compilationFlags.size(), size_t(10));

  ASSERT_EQ(compilationFlags.at(0), "-I"s);
  ASSERT_EQ(compilationFlags.at(1), "foo"s);
  ASSERT_EQ(compilationFlags.at(2), "-I"s);
  ASSERT_EQ(compilationFlags.at(3), "bar"s);
  ASSERT_EQ(compilationFlags.at(4), "-c"s);
  ASSERT_EQ(compilationFlags.at(5), "foobar.cpp"s);
  ASSERT_EQ(compilationFlags.at(6), "-isystem"s);
  ASSERT_EQ(compilationFlags.at(7), "/usr/local/include"s);
  ASSERT_EQ(compilationFlags.at(8), "-isystem"s);
  ASSERT_EQ(compilationFlags.at(9), "/usr/include"s);
}

TEST(CompilationDatabaseFromFile, parsesCompilationFlagsFromClangMJCommandValid) {
  Diagnostics diagnostics;
  auto path = fixtures::junk_detection_compdb_db_produced_from_clang_MJ_valid_sequence_json_path();
  const CompilationDatabase database = CompilationDatabase::fromFile(
      diagnostics, path, "-isystem /usr/local/include -isystem /usr/include", {});

  const std::string file("/tmp/main.cpp");
  auto &[_, compilationFlags] = database.compilationFlagsForFile(file);

  ASSERT_EQ(compilationFlags.size(), size_t(21));
  ASSERT_EQ(compilationFlags.at(0), "-xc++"s);
  ASSERT_EQ(compilationFlags.at(1), "main.cpp"s);
  ASSERT_EQ(compilationFlags.at(2), "-fembed-bitcode=all"s);
  ASSERT_EQ(compilationFlags.at(3), "-g"s);
  ASSERT_EQ(compilationFlags.at(19), "-isystem"s);
  ASSERT_EQ(compilationFlags.at(20), "/usr/include"s);
}

TEST(CompilationDatabaseFromFile, parsesCompilationDatabaseWithEscapedQuotes) {
  Diagnostics diagnostics;
  auto path = fixtures::junk_detection_compdb_db_with_escaped_quotes_json_path();
  const CompilationDatabase database = CompilationDatabase::fromFile(diagnostics, path, "", {});

  const std::string file("/tmp/main.cpp");
  auto &[_, compilationFlags] = database.compilationFlagsForFile(file);

  ASSERT_EQ(compilationFlags.size(), size_t(6));
  ASSERT_EQ(compilationFlags.at(0), "-DQT_CORE_LIB"s);
  ASSERT_EQ(compilationFlags.at(1), "-DQT_TESTCASE_BUILDDIR=\"/src/builds/amd64-linux-mull\""s);
  ASSERT_EQ(compilationFlags.at(2), "-o"s);
  ASSERT_EQ(compilationFlags.at(3),
            "tests/TestingUnitTests/CMakeFiles/TestingUnitTests.dir/ModuleTestSuite.cpp.o"s);
  ASSERT_EQ(compilationFlags.at(4), "-c"s);
}

TEST(CompilationDatabaseFromFile, parsesCompilationFlagsObtainedFromBitcode) {
  const std::string fakeFile = "/opt/fake/file.cpp";

  Diagnostics diagnostics;

  std::unordered_map<std::string, std::string> bitcodeFlags;
  bitcodeFlags[fakeFile] = "fake_compiler -DBITCODE_FLAG=1 -g";

  const CompilationDatabase database =
      CompilationDatabase::fromBuffer(diagnostics, "", "-DEXTRA_FLAG=1", bitcodeFlags);

  auto &[_, compilationFlags] = database.compilationFlagsForFile(fakeFile);

  ASSERT_EQ(compilationFlags.size(), size_t(3));
  ASSERT_EQ(compilationFlags.at(0), "-DBITCODE_FLAG=1"s);
  ASSERT_EQ(compilationFlags.at(1), "-g"s);
  ASSERT_EQ(compilationFlags.at(2), "-DEXTRA_FLAG=1"s);
}
