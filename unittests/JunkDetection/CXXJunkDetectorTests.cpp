#include "Config/Configuration.h"
#include "Config/RawConfig.h"
#include "Program/Program.h"
#include "Mutators/ConditionalsBoundaryMutator.h"
#include "MutationPoint.h"
#include "Toolchain/Compiler.h"
#include "Toolchain/Toolchain.h"
#include "Filter.h"
#include "Testee.h"
#include "MutationsFinder.h"
#include "JunkDetection/CXX/CXXJunkDetector.h"
#include "FixturePaths.h"
#include "ModuleLoader.h"

#include <llvm/IR/Instructions.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>

#include "gtest/gtest.h"

using namespace mull;
using namespace llvm;

TEST(CXXJunkDetector, boundary_mutator) {
  LLVMContext llvmContext;
  ModuleLoader loader;
  auto mullModule = loader.loadModuleAtPath(fixtures::mutators_boundary_module_bc_path(), llvmContext);
  auto module = mullModule->getModule();

  std::vector<std::unique_ptr<MullModule>> modules;
  modules.push_back(std::move(mullModule));
  Program program({}, {}, std::move(modules));
  Configuration configuration;

  std::vector<std::unique_ptr<Mutator>> mutatorss;
  mutatorss.emplace_back(make_unique<ConditionalsBoundaryMutator>());
  MutationsFinder finder(std::move(mutatorss), configuration);
  Filter filter;

  std::vector<std::unique_ptr<Testee>> testees;
  for (auto &function : *module) {
    testees.emplace_back(make_unique<Testee>(&function, nullptr, 1));
  }
  auto mergedTestees = mergeTestees(testees);

  std::vector<MutationPoint *> points = finder.getMutationPoints(program, mergedTestees, filter);

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
  LLVMContext llvmContext;
  ModuleLoader loader;
  auto mullModule = loader.loadModuleAtPath(fixtures::junk_detection_compdb_main_bc_path(), llvmContext);
  auto module = mullModule->getModule();

  std::vector<std::unique_ptr<MullModule>> modules;
  modules.push_back(std::move(mullModule));
  Program program({}, {}, std::move(modules));
  Configuration configuration;

  std::vector<std::unique_ptr<Mutator>> mutators;
  mutators.emplace_back(make_unique<ConditionalsBoundaryMutator>());
  MutationsFinder finder(std::move(mutators), configuration);
  Filter filter;

  std::vector<std::unique_ptr<Testee>> testees;
  for (auto &function : *module) {
    testees.emplace_back(make_unique<Testee>(&function, nullptr, 1));
  }
  auto mergedTestees = mergeTestees(testees);
  std::vector<MutationPoint *> points = finder.getMutationPoints(program, mergedTestees, filter);

  ASSERT_EQ(points.size(), 8U);

  JunkDetectionConfig junkConfig;
  junkConfig.cxxCompDBDirectory = fixtures::junk_detection_compdb_absolute_dir_path();
  CXXJunkDetector detector(junkConfig);
  std::vector<MutationPoint *> nonJunkMutationPoints;
  for (auto point: points) {
    if (!detector.isJunk(point)) {
      nonJunkMutationPoints.push_back(point);
    }
  }

  ASSERT_EQ(nonJunkMutationPoints.size(), 7U);
}

TEST(CXXJunkDetector, DISABLED_compdb_relative_paths) {
  LLVMContext llvmContext;
  ModuleLoader loader;
  auto mullModule = loader.loadModuleAtPath(fixtures::junk_detection_compdb_main_bc_path(), llvmContext);
  auto module = mullModule->getModule();

  std::vector<std::unique_ptr<MullModule>> modules;
  modules.push_back(std::move(mullModule));
  Program program({}, {}, std::move(modules));
  Configuration configuration;

  std::vector<std::unique_ptr<Mutator>> mutatorss;
  mutatorss.emplace_back(make_unique<ConditionalsBoundaryMutator>());
  MutationsFinder finder(std::move(mutatorss), configuration);
  Filter filter;

  std::vector<std::unique_ptr<Testee>> testees;
  for (auto &function : *module) {
    testees.emplace_back(make_unique<Testee>(&function, nullptr, 1));
  }
  auto mergedTestees = mergeTestees(testees);

  std::vector<MutationPoint *> points = finder.getMutationPoints(program, mergedTestees, filter);

  ASSERT_EQ(points.size(), 8U);

  JunkDetectionConfig junkConfig;
  junkConfig.cxxCompDBDirectory = fixtures::junk_detection_compdb_relative_dir_path();
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
  LLVMContext llvmContext;
  ModuleLoader loader;
  auto mullModule = loader.loadModuleAtPath(fixtures::junk_detection_compdb_main_bc_path(), llvmContext);
  auto module = mullModule->getModule();

  std::vector<std::unique_ptr<MullModule>> modules;
  modules.push_back(std::move(mullModule));
  Program program({}, {}, std::move(modules));
  Configuration configuration;

  std::vector<std::unique_ptr<Mutator>> mutatorss;
  mutatorss.emplace_back(make_unique<ConditionalsBoundaryMutator>());
  MutationsFinder finder(std::move(mutatorss), configuration);
  Filter filter;

  std::vector<std::unique_ptr<Testee>> testees;
  for (auto &function : *module) {
    testees.emplace_back(make_unique<Testee>(&function, nullptr, 1));
  }
  auto mergedTestees = mergeTestees(testees);

  std::vector<MutationPoint *> points = finder.getMutationPoints(program, mergedTestees, filter);

  ASSERT_EQ(points.size(), 8U);

  JunkDetectionConfig junkConfig;
  junkConfig.cxxCompilationFlags = std::string("-I ") + fixtures::junk_detection_compdb_include__path();

  CXXJunkDetector detector(junkConfig);
  std::vector<MutationPoint *> nonJunkMutationPoints;
  for (auto point: points) {
    if (!detector.isJunk(point)) {
      nonJunkMutationPoints.push_back(point);
    }
  }

  ASSERT_EQ(nonJunkMutationPoints.size(), 7U);
}
