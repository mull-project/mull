#include "FixturePaths.h"
#include "mull/BitcodeLoader.h"
#include "mull/Config/Configuration.h"
#include "mull/Filters/FilePathFilter.h"
#include "mull/Filters/NoDebugInfoFilter.h"
#include "mull/MutationsFinder.h"
#include "mull/Program/Program.h"
#include <mull/Mutators/CXX/ArithmeticMutators.h>

#include <gtest/gtest.h>
#include <llvm/IR/LLVMContext.h>
#include <mull/Diagnostics/Diagnostics.h>

using namespace mull;

TEST(NoDebugInfoFilter, withDebugInfo) {
  Diagnostics diagnostics;
  llvm::LLVMContext context;
  BitcodeLoader loader;
  auto path = fixtures::mutation_filters_no_debug_filter_with_debug_bc_path();
  auto bitcode = loader.loadBitcodeAtPath(path, context, diagnostics);
  std::vector<MutationPoint *> points;

  cxx::AddToSub mutator;
  for (auto &function : bitcode->getModule()->functions()) {
    FunctionUnderTest functionUnderTest(&function, nullptr, 0);
    functionUnderTest.selectInstructions({});
    auto mutants = mutator.getMutations(bitcode.get(), functionUnderTest);
    std::copy(mutants.begin(), mutants.end(), std::back_inserter(points));
  }

  ASSERT_NE(points.size(), size_t(0));

  std::vector<MutationPoint *> filteredPoints;
  NoDebugInfoFilter mutationFilter;
  for (auto point : points) {
    if (!mutationFilter.shouldSkip(point)) {
      filteredPoints.push_back(point);
    }
  }

  ASSERT_EQ(points.size(), filteredPoints.size());
}

TEST(NoDebugInfoFilter, withouDebugInfo) {
  Diagnostics diagnostics;
  llvm::LLVMContext context;
  BitcodeLoader loader;
  auto path = fixtures::mutation_filters_no_debug_filter_without_debug_bc_path();
  auto bitcode = loader.loadBitcodeAtPath(path, context, diagnostics);

  std::vector<MutationPoint *> points;
  cxx::AddToSub mutator;
  for (auto &function : bitcode->getModule()->functions()) {
    FunctionUnderTest functionUnderTest(&function, nullptr, 0);
    functionUnderTest.selectInstructions({});
    auto mutants = mutator.getMutations(bitcode.get(), functionUnderTest);
    std::copy(mutants.begin(), mutants.end(), std::back_inserter(points));
  }

  ASSERT_NE(points.size(), size_t(0));

  std::vector<MutationPoint *> filteredPoints;
  NoDebugInfoFilter mutationFilter;
  for (auto point : points) {
    if (!mutationFilter.shouldSkip(point)) {
      filteredPoints.push_back(point);
    }
  }

  ASSERT_EQ(filteredPoints.size(), size_t(0));
}

TEST(FilePathFilter, doesNotFilterEmpty) {
  Diagnostics diagnostics;
  llvm::LLVMContext context;
  BitcodeLoader loader;
  auto path = fixtures::mutation_filters_file_path_some_test_file_name_bc_path();
  auto bitcode = loader.loadBitcodeAtPath(path, context, diagnostics);

  std::vector<MutationPoint *> points;
  cxx::AddToSub mutator;
  for (auto &function : bitcode->getModule()->functions()) {
    FunctionUnderTest functionUnderTest(&function, nullptr, 0);
    functionUnderTest.selectInstructions({});
    auto mutants = mutator.getMutations(bitcode.get(), functionUnderTest);
    std::copy(mutants.begin(), mutants.end(), std::back_inserter(points));
  }

  ASSERT_NE(points.size(), size_t(0));

  std::vector<MutationPoint *> filteredPoints;
  FilePathFilter filePathFilter;
  for (auto point : points) {
    if (!filePathFilter.shouldSkip(point)) {
      filteredPoints.push_back(point);
    }
  }

  ASSERT_EQ(filteredPoints.size(), points.size());
}

TEST(FilePathFilter, doesNotFilterMismatch) {
  Diagnostics diagnostics;
  llvm::LLVMContext context;
  BitcodeLoader loader;
  auto path = fixtures::mutation_filters_file_path_some_test_file_name_bc_path();
  auto bitcode = loader.loadBitcodeAtPath(path, context, diagnostics);

  std::vector<MutationPoint *> points;
  cxx::AddToSub mutator;
  for (auto &function : bitcode->getModule()->functions()) {
    FunctionUnderTest functionUnderTest(&function, nullptr, 0);
    functionUnderTest.selectInstructions({});
    auto mutants = mutator.getMutations(bitcode.get(), functionUnderTest);
    std::copy(mutants.begin(), mutants.end(), std::back_inserter(points));
  }

  ASSERT_NE(points.size(), size_t(0));

  std::vector<MutationPoint *> filteredPoints;
  FilePathFilter filePathFilter;
  filePathFilter.exclude("irrelevant");
  for (auto point : points) {
    if (!filePathFilter.shouldSkip(point)) {
      filteredPoints.push_back(point);
    }
  }

  ASSERT_EQ(filteredPoints.size(), points.size());
}

TEST(FilePathFilter, filtersPlainString) {
  Diagnostics diagnostics;
  llvm::LLVMContext context;
  BitcodeLoader loader;
  auto path = fixtures::mutation_filters_file_path_some_test_file_name_bc_path();
  auto bitcode = loader.loadBitcodeAtPath(path, context, diagnostics);

  std::vector<MutationPoint *> points;
  cxx::AddToSub mutator;
  for (auto &function : bitcode->getModule()->functions()) {
    FunctionUnderTest functionUnderTest(&function, nullptr, 0);
    functionUnderTest.selectInstructions({});
    auto mutants = mutator.getMutations(bitcode.get(), functionUnderTest);
    std::copy(mutants.begin(), mutants.end(), std::back_inserter(points));
  }

  ASSERT_NE(points.size(), size_t(0));

  std::vector<MutationPoint *> filteredPoints;
  FilePathFilter filePathFilter;
  filePathFilter.exclude("some_test");
  for (auto point : points) {
    if (!filePathFilter.shouldSkip(point)) {
      filteredPoints.push_back(point);
    }
  }

  ASSERT_EQ(filteredPoints.size(), size_t(0));
}

TEST(FilePathFilter, filtersWithRegex) {
  Diagnostics diagnostics;
  llvm::LLVMContext context;
  BitcodeLoader loader;
  auto path = fixtures::mutation_filters_file_path_some_test_file_name_bc_path();
  auto bitcode = loader.loadBitcodeAtPath(path, context, diagnostics);

  std::vector<MutationPoint *> points;
  cxx::AddToSub mutator;
  for (auto &function : bitcode->getModule()->functions()) {
    FunctionUnderTest functionUnderTest(&function, nullptr, 0);
    functionUnderTest.selectInstructions({});
    auto mutants = mutator.getMutations(bitcode.get(), functionUnderTest);
    std::copy(mutants.begin(), mutants.end(), std::back_inserter(points));
  }

  ASSERT_NE(points.size(), size_t(0));

  std::vector<MutationPoint *> filteredPoints;
  FilePathFilter filePathFilter;
  filePathFilter.exclude("some.*name");
  for (auto point : points) {
    if (!filePathFilter.shouldSkip(point)) {
      filteredPoints.push_back(point);
    }
  }

  ASSERT_EQ(filteredPoints.size(), size_t(0));
}
