#include <gtest/gtest.h>

#include "FixturePaths.h"
#include "mull/BitcodeLoader.h"
#include "mull/Config/Configuration.h"
#include "mull/Filter.h"
#include "mull/MutationFilters/NoDebugInfoFilter.h"
#include "mull/MutationsFinder.h"
#include "mull/Mutators/MathAddMutator.h"
#include "mull/Program/Program.h"

#include <llvm/IR/LLVMContext.h>

using namespace mull;

TEST(NoDebugInfoFilter, withDebugInfo) {
  llvm::LLVMContext context;
  BitcodeLoader loader;
  auto bitcodeFile = loader.loadBitcodeAtPath(
      fixtures::mutation_filters_no_debug_filter_with_debug_bc_path(), context);
  auto module = bitcodeFile->getModule();

  std::vector<std::unique_ptr<Bitcode>> bitcode;
  bitcode.push_back(std::move(bitcodeFile));
  Program program({}, {}, std::move(bitcode));
  Configuration configuration;

  std::vector<std::unique_ptr<Mutator>> mutators;
  mutators.emplace_back(llvm::make_unique<MathAddMutator>());
  MutationsFinder finder(std::move(mutators), configuration);
  Filter filter;

  std::vector<std::unique_ptr<Testee>> testees;
  for (auto &function : *module) {
    testees.emplace_back(llvm::make_unique<Testee>(&function, nullptr, 1));
  }
  auto mergedTestees = mergeTestees(testees);
  std::vector<MutationPoint *> points =
      finder.getMutationPoints(program, mergedTestees, filter);

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
  auto bitcodeFile = loader.loadBitcodeAtPath(
      fixtures::mutation_filters_no_debug_filter_without_debug_bc_path(),
      context);
  auto module = bitcodeFile->getModule();

  std::vector<std::unique_ptr<Bitcode>> bitcode;
  bitcode.push_back(std::move(bitcodeFile));
  Program program({}, {}, std::move(bitcode));
  Configuration configuration;

  std::vector<std::unique_ptr<Mutator>> mutators;
  mutators.emplace_back(llvm::make_unique<MathAddMutator>());
  MutationsFinder finder(std::move(mutators), configuration);
  Filter filter;

  std::vector<std::unique_ptr<Testee>> testees;
  for (auto &function : *module) {
    testees.emplace_back(llvm::make_unique<Testee>(&function, nullptr, 1));
  }
  auto mergedTestees = mergeTestees(testees);
  std::vector<MutationPoint *> points =
      finder.getMutationPoints(program, mergedTestees, filter);

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
