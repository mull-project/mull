#include "Config.h"
#include "Context.h"
#include "Mutators/ConditionalsBoundaryMutator.h"
#include "MutationPoint.h"
#include "TestModuleFactory.h"
#include "Toolchain/Compiler.h"
#include "Toolchain/Toolchain.h"
#include "Filter.h"
#include "Testee.h"
#include "MutationsFinder.h"
#include "JunkDetection/CXX/CXXJunkDetector.h"

#include <llvm/IR/Instructions.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>

#include "gtest/gtest.h"

using namespace mull;
using namespace llvm;

static TestModuleFactory TestModuleFactory;

TEST(CXXJunkDetector, boundary_mutator) {
  auto mullModule = TestModuleFactory.create_ConditionalsBoundaryMutator_Module();
  auto module = mullModule->getModule();

  Context mullContext;
  mullContext.addModule(std::move(mullModule));
  Config config;
  config.normalizeParallelizationConfig();

  std::vector<std::unique_ptr<Mutator>> mutatorss;
  mutatorss.emplace_back(make_unique<ConditionalsBoundaryMutator>());
  MutationsFinder finder(std::move(mutatorss), config);
  Filter filter;

  std::vector<std::unique_ptr<Testee>> testees;
  for (auto &function : *module) {
    testees.emplace_back(make_unique<Testee>(&function, nullptr, 1));
  }
  auto mergedTestees = mergeTestees(testees);

  std::vector<MutationPoint *> points = finder.getMutationPoints(mullContext, mergedTestees, filter);

  ASSERT_EQ(points.size(), 7U);

  JunkDetectionConfig junkConfig;

  CXXJunkDetector detector(junkConfig);
  std::vector<MutationPoint *> nonJunkMutationPoints;
  for (auto point: points) {
    if (!detector.isJunk(point)) {
      nonJunkMutationPoints.push_back(point);
    }
  }

  ASSERT_EQ(nonJunkMutationPoints.size(), 6U);
}

TEST(CXXJunkDetector, compdb_absolute_paths) {
  auto mullModule = TestModuleFactory.create_CompilationDatabase_AbsolutePath_Module();
  auto module = mullModule->getModule();

  Context mullContext;
  mullContext.addModule(std::move(mullModule));
  Config config;
  config.normalizeParallelizationConfig();

  std::vector<std::unique_ptr<Mutator>> mutatorss;
  mutatorss.emplace_back(make_unique<ConditionalsBoundaryMutator>());
  MutationsFinder finder(std::move(mutatorss), config);
  Filter filter;

  std::vector<std::unique_ptr<Testee>> testees;
  for (auto &function : *module) {
    testees.emplace_back(make_unique<Testee>(&function, nullptr, 1));
  }
  auto mergedTestees = mergeTestees(testees);
  std::vector<MutationPoint *> points = finder.getMutationPoints(mullContext, mergedTestees, filter);

  ASSERT_EQ(points.size(), 8U);

  JunkDetectionConfig junkConfig;
  junkConfig.cxxCompDBDirectory = TestModuleFactory.CompilationDatabase_AbsolutePath_Directory();
  CXXJunkDetector detector(junkConfig);
  std::vector<MutationPoint *> nonJunkMutationPoints;
  for (auto point: points) {
    if (!detector.isJunk(point)) {
      nonJunkMutationPoints.push_back(point);
    }
  }

  ASSERT_EQ(nonJunkMutationPoints.size(), 7U);
}

TEST(CXXJunkDetector, compdb_relative_paths) {
  auto mullModule = TestModuleFactory.create_CompilationDatabase_RelativePath_Module();
  auto module = mullModule->getModule();

  Context mullContext;
  mullContext.addModule(std::move(mullModule));
  Config config;
  config.normalizeParallelizationConfig();

  std::vector<std::unique_ptr<Mutator>> mutatorss;
  mutatorss.emplace_back(make_unique<ConditionalsBoundaryMutator>());
  MutationsFinder finder(std::move(mutatorss), config);
  Filter filter;

  std::vector<std::unique_ptr<Testee>> testees;
  for (auto &function : *module) {
    testees.emplace_back(make_unique<Testee>(&function, nullptr, 1));
  }
  auto mergedTestees = mergeTestees(testees);

  std::vector<MutationPoint *> points = finder.getMutationPoints(mullContext, mergedTestees, filter);

  ASSERT_EQ(points.size(), 8U);

  JunkDetectionConfig junkConfig;
  junkConfig.cxxCompDBDirectory = TestModuleFactory.CompilationDatabase_RelativePath_Directory();
  CXXJunkDetector detector(junkConfig);
  std::vector<MutationPoint *> nonJunkMutationPoints;
  for (auto point: points) {
    if (!detector.isJunk(point)) {
      nonJunkMutationPoints.push_back(point);
    }
  }

  ASSERT_EQ(nonJunkMutationPoints.size(), 7U);
}

TEST(CXXJunkDetector, no_compdb) {
  auto mullModule = TestModuleFactory.create_CompilationDatabase_RelativePath_Module();
  auto module = mullModule->getModule();

  Context mullContext;
  mullContext.addModule(std::move(mullModule));
  Config config;
  config.normalizeParallelizationConfig();

  std::vector<std::unique_ptr<Mutator>> mutatorss;
  mutatorss.emplace_back(make_unique<ConditionalsBoundaryMutator>());
  MutationsFinder finder(std::move(mutatorss), config);
  Filter filter;

  std::vector<std::unique_ptr<Testee>> testees;
  for (auto &function : *module) {
    testees.emplace_back(make_unique<Testee>(&function, nullptr, 1));
  }
  auto mergedTestees = mergeTestees(testees);

  std::vector<MutationPoint *> points = finder.getMutationPoints(mullContext, mergedTestees, filter);

  ASSERT_EQ(points.size(), 8U);

  JunkDetectionConfig junkConfig;
  junkConfig.cxxCompilationFlags = "-I include";
  CXXJunkDetector detector(junkConfig);
  std::vector<MutationPoint *> nonJunkMutationPoints;
  for (auto point: points) {
    if (!detector.isJunk(point)) {
      nonJunkMutationPoints.push_back(point);
    }
  }

  ASSERT_EQ(nonJunkMutationPoints.size(), 7U);
}
