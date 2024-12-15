#include "mull/Filters/GitDiffReader.h"

#include "mull/Diagnostics/Diagnostics.h"

#include <gtest/gtest.h>

using namespace mull;

TEST(GitDiffReaderTest, 01_SingleFile_SingleLine) {
  Diagnostics diagnostics;
  GitDiffReader gitDiffReader(diagnostics, "/tmp/repo");

  const std::string diff = std::string(R"(
diff --git a/lib/Driver.cpp b/lib/Driver.cpp
index 768daa6b..5ebc9315 100644
--- a/lib/Driver.cpp
+++ b/lib/Driver.cpp
@@ -93,0 +94 @@ std::vector<MutationPoint *> Driver::filterMutations(std::vector<MutationPoint *
+
)");

  GitDiffInfo gitDiffInfo = gitDiffReader.parseDiffContent(diff);

  ASSERT_EQ(gitDiffInfo.size(), 1);

  const GitDiffSourceFileRanges &fileRanges = gitDiffInfo["/tmp/repo/lib/Driver.cpp"];
  ASSERT_EQ(fileRanges.size(), 1);
  ASSERT_EQ(fileRanges[0].first, 94);
  ASSERT_EQ(fileRanges[0].second, 1);
}

TEST(GitDiffReaderTest, 02_SingleFile_TwoLineDiff) {
  Diagnostics diagnostics;
  GitDiffReader gitDiffReader(diagnostics, "/tmp/repo");

  const std::string diff = std::string(R"(
diff --git a/lib/Driver.cpp b/lib/Driver.cpp
index 768daa6b..5ebc9315 100644
--- a/lib/Driver.cpp
+++ b/lib/Driver.cpp
@@ -93,0 +94,2 @@ std::vector<MutationPoint *> Driver::filterMutations(std::vector<MutationPoint *
+
+
)");

  GitDiffInfo gitDiffInfo = gitDiffReader.parseDiffContent(diff);

  ASSERT_EQ(gitDiffInfo.size(), 1);

  const GitDiffSourceFileRanges &fileRanges = gitDiffInfo["/tmp/repo/lib/Driver.cpp"];
  ASSERT_EQ(fileRanges.size(), 1);
  ASSERT_EQ(fileRanges[0].first, 94);
  ASSERT_EQ(fileRanges[0].second, 2);
}

TEST(GitDiffReaderTest, 03_SingleFile_ThreeRanges) {
  Diagnostics diagnostics;
  GitDiffReader gitDiffReader(diagnostics, "/tmp/repo");

  const std::string diff = std::string(R"(
diff --git a/lib/Driver.cpp b/lib/Driver.cpp
index 768daa6b..045b1d03 100644
--- a/lib/Driver.cpp
+++ b/lib/Driver.cpp
@@ -38,0 +39,2 @@ std::unique_ptr<Result> Driver::run() {
+
+
@@ -50,0 +53,2 @@ std::unique_ptr<Result> Driver::run() {
+
+
@@ -93,0 +98,2 @@ std::vector<MutationPoint *> Driver::filterMutations(std::vector<MutationPoint *
+
+
)");

  GitDiffInfo gitDiffInfo = gitDiffReader.parseDiffContent(diff);

  ASSERT_EQ(gitDiffInfo.size(), 1);

  const GitDiffSourceFileRanges &fileRanges = gitDiffInfo["/tmp/repo/lib/Driver.cpp"];
  ASSERT_EQ(fileRanges.size(), 3);
  ASSERT_EQ(fileRanges[0].first, 39);
  ASSERT_EQ(fileRanges[0].second, 2);
  ASSERT_EQ(fileRanges[1].first, 53);
  ASSERT_EQ(fileRanges[1].second, 2);
  ASSERT_EQ(fileRanges[2].first, 98);
  ASSERT_EQ(fileRanges[2].second, 2);
}

TEST(GitDiffReaderTest, 04_ThreeDifferentFilesOneRangeEach) {
  Diagnostics diagnostics;
  GitDiffReader gitDiffReader(diagnostics, "/tmp/repo");

  const std::string diff = std::string(R"(
diff --git a/lib/Bitcode.cpp b/lib/Bitcode.cpp
index f04c8da3..09e7ad29 100644
--- a/lib/Bitcode.cpp
+++ b/lib/Bitcode.cpp
@@ -27,0 +28 @@ llvm::Module *Bitcode::getModule() const {
+
diff --git a/lib/Driver.cpp b/lib/Driver.cpp
index 768daa6b..16124670 100644
--- a/lib/Driver.cpp
+++ b/lib/Driver.cpp
@@ -93,0 +94,2 @@ std::vector<MutationPoint *> Driver::filterMutations(std::vector<MutationPoint *
+
+
diff --git a/lib/Path.cpp b/lib/Path.cpp
index 31b74f91..ea3c36a4 100644
--- a/lib/Path.cpp
+++ b/lib/Path.cpp
@@ -8,0 +9 @@ std::string mull::absoluteFilePath(const std::string &directory, const std::stri
+
)");

  GitDiffInfo gitDiffInfo = gitDiffReader.parseDiffContent(diff);

  ASSERT_EQ(gitDiffInfo.size(), 3);

  const GitDiffSourceFileRanges &file1Ranges = gitDiffInfo["/tmp/repo/lib/Bitcode.cpp"];
  ASSERT_EQ(file1Ranges.size(), 1);
  ASSERT_EQ(file1Ranges[0].first, 28);
  ASSERT_EQ(file1Ranges[0].second, 1);

  const GitDiffSourceFileRanges &file2Ranges = gitDiffInfo["/tmp/repo/lib/Driver.cpp"];
  ASSERT_EQ(file2Ranges.size(), 1);
  ASSERT_EQ(file2Ranges[0].first, 94);
  ASSERT_EQ(file2Ranges[0].second, 2);

  const GitDiffSourceFileRanges &file3Ranges = gitDiffInfo["/tmp/repo/lib/Path.cpp"];
  ASSERT_EQ(file3Ranges.size(), 1);
  ASSERT_EQ(file3Ranges[0].first, 9);
  ASSERT_EQ(file3Ranges[0].second, 1);
}
