#include "FixturePaths.h"
#include "mull/BitcodeLoader.h"
#include "mull/Config/Configuration.h"
#include "mull/Filter.h"
#include "mull/MutationFilters/FilePathFilter.h"
#include "mull/MutationFilters/NoDebugInfoFilter.h"
#include "mull/MutationsFinder.h"
#include "mull/Program/Program.h"
#include <mull/Mutators/CXX/ArithmeticMutators.h>

#include <gtest/gtest.h>
#include <llvm/IR/LLVMContext.h>

using namespace mull;

TEST(NoDebugInfoFilter, withDebugInfo) {
  llvm::LLVMContext context;
  BitcodeLoader loader;
  auto path = fixtures::mutation_filters_no_debug_filter_with_debug_bc_path();
  auto bitcode = loader.loadBitcodeAtPath(path, context);
  std::vector<MutationPoint *> points;

  cxx::AddToSub mutator;
  for (auto &function : bitcode->getModule()->functions()) {
    auto mutants = mutator.getMutations(bitcode.get(), &function);
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
  llvm::LLVMContext context;
  BitcodeLoader loader;
  auto path =
      fixtures::mutation_filters_no_debug_filter_without_debug_bc_path();
  auto bitcode = loader.loadBitcodeAtPath(path, context);

  std::vector<MutationPoint *> points;
  cxx::AddToSub mutator;
  for (auto &function : bitcode->getModule()->functions()) {
    auto mutants = mutator.getMutations(bitcode.get(), &function);
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
  llvm::LLVMContext context;
  BitcodeLoader loader;
  auto path =
      fixtures::mutation_filters_file_path_some_test_file_name_bc_path();
  auto bitcode = loader.loadBitcodeAtPath(path, context);

  std::vector<MutationPoint *> points;
  cxx::AddToSub mutator;
  for (auto &function : bitcode->getModule()->functions()) {
    auto mutants = mutator.getMutations(bitcode.get(), &function);
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
  llvm::LLVMContext context;
  BitcodeLoader loader;
  auto path =
      fixtures::mutation_filters_file_path_some_test_file_name_bc_path();
  auto bitcode = loader.loadBitcodeAtPath(path, context);

  std::vector<MutationPoint *> points;
  cxx::AddToSub mutator;
  for (auto &function : bitcode->getModule()->functions()) {
    auto mutants = mutator.getMutations(bitcode.get(), &function);
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
  llvm::LLVMContext context;
  BitcodeLoader loader;
  auto path =
      fixtures::mutation_filters_file_path_some_test_file_name_bc_path();
  auto bitcode = loader.loadBitcodeAtPath(path, context);

  std::vector<MutationPoint *> points;
  cxx::AddToSub mutator;
  for (auto &function : bitcode->getModule()->functions()) {
    auto mutants = mutator.getMutations(bitcode.get(), &function);
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
  llvm::LLVMContext context;
  BitcodeLoader loader;
  auto path =
      fixtures::mutation_filters_file_path_some_test_file_name_bc_path();
  auto bitcode = loader.loadBitcodeAtPath(path, context);

  std::vector<MutationPoint *> points;
  cxx::AddToSub mutator;
  for (auto &function : bitcode->getModule()->functions()) {
    auto mutants = mutator.getMutations(bitcode.get(), &function);
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
