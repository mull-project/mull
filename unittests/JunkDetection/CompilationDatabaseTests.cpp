#include "FixturePaths.h"
#include "JunkDetection/CXX/CompilationDatabase.h"

#include <clang/Basic/Version.h>

#include <gtest/gtest.h>

using namespace mull;

TEST(CompilationDatabaseFromCompilationFlags, returnsAllFlags) {
  const CompilationDatabase database(
      CompilationDatabase::Path(""),
      CompilationDatabase::Flags("-I     /usr/foo/include   "));

  const std::string filepath("foobar");
  const auto flags = database.compilationFlagsForFile(filepath);
  ASSERT_EQ(flags.size(), size_t(2));
}

TEST(CompilationDatabaseFromCompilationFlags, returnsAllFlagsForAnyFile) {
  const CompilationDatabase database(
      CompilationDatabase::Path(""),
      CompilationDatabase::Flags("-I /usr/foo/include   "));

  const std::string foo("foo");
  const std::string bar("bar");

  const auto fooFlags = database.compilationFlagsForFile(foo);
  const auto barFlags = database.compilationFlagsForFile(bar);

  ASSERT_EQ(fooFlags.size(), barFlags.size());

  for (size_t index(0); index < fooFlags.size(); index++) {
    auto &fooFlag = fooFlags.at(index);
    auto &barFlag = barFlags.at(index);

    ASSERT_EQ(fooFlag, barFlag);
  }
}

TEST(CompilationDatabaseFromCompilationFlags, cleansUpFlags) {
  const std::vector<std::string> testFlags(
      {"-c", "-c foo.cpp", "-c     foo.cpp"});

  const std::string file("foobar.cpp");
  for (auto &flags : testFlags) {
    const CompilationDatabase database(CompilationDatabase::Path(""),
                                       CompilationDatabase::Flags(flags));

    auto compilationFlags = database.compilationFlagsForFile(file);
    ASSERT_EQ(compilationFlags.size(), size_t(0));
  }
}

TEST(CompilationDatabaseFromCompilationFlags, cleansUpMixedFlags) {
  const std::vector<std::string> testFlags(
      {"-I foo -I bar -c foo.cpp", "-c foo.cpp -I foo -I bar",
       "-I foo -c foo.cpp -I bar", "-I foo -I bar -c"});

  const std::string file("foobar.cpp");
  for (auto &flags : testFlags) {
    const CompilationDatabase database(CompilationDatabase::Path(""),
                                       CompilationDatabase::Flags(flags));

    auto compilationFlags = database.compilationFlagsForFile(file);
    ASSERT_EQ(compilationFlags.size(), size_t(4));

    ASSERT_EQ(compilationFlags.at(0), std::string("-I"));
    ASSERT_EQ(compilationFlags.at(1), std::string("foo"));
    ASSERT_EQ(compilationFlags.at(2), std::string("-I"));
    ASSERT_EQ(compilationFlags.at(3), std::string("bar"));
  }
}

TEST(CompilationDatabaseFromFile, loadsFromValidFiles) {
  const std::vector<std::string> databasePaths(
      {fixtures::junk_detection_compdb_db_with_arguments_json_path(),
       fixtures::junk_detection_compdb_db_with_commands_json_path(),
       fixtures::junk_detection_compdb_db_with_output_json_path()});

  const std::string file("foobar.cpp");
  for (auto &path : databasePaths) {
    const CompilationDatabase database(CompilationDatabase::Path(path),
                                       CompilationDatabase::Flags(""));

    auto compilationFlags = database.compilationFlagsForFile(file);
    ASSERT_EQ(compilationFlags.size(), size_t(4));

    ASSERT_EQ(compilationFlags.at(0), std::string("-I"));
    ASSERT_EQ(compilationFlags.at(1), std::string("foo"));
    ASSERT_EQ(compilationFlags.at(2), std::string("-I"));
    ASSERT_EQ(compilationFlags.at(3), std::string("bar"));
  }
}

TEST(CompilationDatabaseFromFile, includesInternalHeaderSearchPaths) {
  auto path =
      fixtures::junk_detection_compdb_db_with_fullpath_compiler_json_path();
  const CompilationDatabase database(CompilationDatabase::Path(path),
                                     CompilationDatabase::Flags(""));

  const std::string file("foobar.cpp");
  auto compilationFlags = database.compilationFlagsForFile(file);
  ASSERT_EQ(compilationFlags.size(), size_t(8));

  ASSERT_EQ(compilationFlags.at(0), std::string("-I"));
  ASSERT_EQ(compilationFlags.at(1), std::string("foo"));
  ASSERT_EQ(compilationFlags.at(2), std::string("-I"));
  ASSERT_EQ(compilationFlags.at(3), std::string("bar"));
  ASSERT_EQ(compilationFlags.at(4), std::string("-I"));
  ASSERT_EQ(compilationFlags.at(5), std::string("/opt/include/c++/v1"));
  ASSERT_EQ(compilationFlags.at(6), std::string("-I"));
  ASSERT_EQ(compilationFlags.at(7),
            std::string("/opt/lib/clang/" CLANG_VERSION_STRING "/include"));
}
