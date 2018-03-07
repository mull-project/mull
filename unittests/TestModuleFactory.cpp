
#include "TestModuleFactory.h"

#include "Logger.h"

#include <llvm/AsmParser/Parser.h>
#include <llvm/Bitcode/ReaderWriter.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/Debug.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/SourceMgr.h>

#include <iostream>
#include <fstream>
#include <string>
#include <unistd.h>

static LLVMContext GlobalCtx;

static inline bool fileExists(const std::string& name) {
  std::ifstream f(name.c_str());
  return f.good();
}

static std::string createFixture(const char *fixtureName) {
  std::string fixtureFullPath = TestModuleFactory::fixturePath(fixtureName);

  if (fileExists(fixtureFullPath) == false) {
    mull::Logger::debug() << "Could not find a fixture at path: "
                          << fixtureFullPath << '\n';

    exit(1);
  }

  std::ifstream file(fixtureFullPath);
  std::string str;
  std::string file_contents;
  while (std::getline(file, str)) {
    file_contents += str;
    file_contents.push_back('\n');
  }

  return file_contents;
}

static std::unique_ptr<Module> parseIR(const char *rawBitcode) {
  SMDiagnostic error;
  auto module = parseAssemblyString(rawBitcode, error, GlobalCtx);

  /// FIXME: is there another way to check for errors?
  if (error.getMessage().empty() == false) {
    error.print("test", dbgs());
  }

  assert(module && "Expected module to be parsed correctly");

  assert(!llvm::verifyModule(*module, &dbgs()));

  return module;
}

static std::unique_ptr<MullModule>
createModuleFromBitcode(const char *fixtureName,
                        const char *moduleIdentifier) {
  std::string fixtureFullPath = TestModuleFactory::fixturePath(fixtureName);

  auto bufferOrError = MemoryBuffer::getFile(fixtureFullPath);
  if (!bufferOrError) {
    Logger::error() << "TestModuleFactory> Can't load module " << fixtureFullPath << '\n';
    abort();
  }

  auto llvmModule = parseBitcodeFile(bufferOrError->get()->getMemBufferRef(), GlobalCtx);
  if (!llvmModule) {
    Logger::error() << "TestModuleFactory> Can't load module " << fixtureFullPath << '\n';
    abort();
  }

  auto module = make_unique<MullModule>(std::move(llvmModule.get()),
                                        "fake_hash",
                                        fixtureFullPath);
  return module;
}

std::string TestModuleFactory::fixturePath(const char *fixtureName) {
  char cFixtureFullPath[256];

  getcwd(cFixtureFullPath, 255);
  strcat(cFixtureFullPath, "/fixtures/");
  strcat(cFixtureFullPath, fixtureName);

  std::string fixtureFullPath(cFixtureFullPath);

  return fixtureFullPath;
}

std::unique_ptr<MullModule>
TestModuleFactory::createModule(const char *fixtureName,
                                const char *moduleIdentifier) {
  std::string contents = createFixture(fixtureName);

  auto module = parseIR(contents.c_str());

  module->setModuleIdentifier(moduleIdentifier);

  return make_unique<MullModule>(std::move(module), "fake_hash", "fake_path");
}

#pragma mark - Mutators

#pragma mark - Math Mutators

std::unique_ptr<MullModule> TestModuleFactory::create_SimpleTest_MathSub_Module() {
  const char *fixture = "simple_test/mutators/math_sub/math_sub.bc";
  return createModuleFromBitcode(fixture, fixture);
}

std::unique_ptr<MullModule> TestModuleFactory::create_SimpleTest_MathMul_Module() {
  const char *fixture = "simple_test/mutators/math_mul/math_mul.bc";
  return createModuleFromBitcode(fixture, fixture);
}

std::unique_ptr<MullModule> TestModuleFactory::create_SimpleTest_MathDiv_Module() {
  const char *fixture = "simple_test/mutators/math_div/math_div.bc";
  return createModuleFromBitcode(fixture, fixture);
}

#pragma mark - Negate Condition

std::unique_ptr<MullModule> TestModuleFactory::create_SimpleTest_NegateCondition_Tester_Module() {
  const char *fixture = "simple_test/mutators/negate_condition/tester.bc";
  return createModuleFromBitcode(fixture, fixture);
}

std::unique_ptr<MullModule> TestModuleFactory::create_SimpleTest_NegateCondition_Testee_Module() {
  const char *fixture = "simple_test/mutators/negate_condition/testee.bc";
  return createModuleFromBitcode(fixture, fixture);
}

#pragma mark - Remove Void Function

std::unique_ptr<MullModule> TestModuleFactory::create_SimpleTest_RemoveVoidFunction_Tester_Module() {
  const char *fixture = "simple_test/mutators/remove_void_function/tester.bc";
  return createModuleFromBitcode(fixture, fixture);
}

std::unique_ptr<MullModule> TestModuleFactory::create_SimpleTest_RemoveVoidFunction_Testee_Module() {
  const char *fixture = "simple_test/mutators/remove_void_function/testee.bc";
  return createModuleFromBitcode(fixture, fixture);
}

#pragma mark - AND <-> OR Replacement

std::unique_ptr<MullModule> TestModuleFactory::create_SimpleTest_ANDORReplacement_Module() {
  const char *fixture = "simple_test/mutators/and_or_replacement/test_and_or_operators.bc";
  return createModuleFromBitcode(fixture, fixture);
}

std::unique_ptr<MullModule> TestModuleFactory::create_SimpleTest_ANDORReplacement_CPPContent_Module() {
  const char *fixture = "simple_test/mutators/and_or_replacement_cpp/test_and_or_operators.bc";
  return createModuleFromBitcode(fixture, fixture);
}

#pragma mark - Scalar Value

std::unique_ptr<MullModule> TestModuleFactory::create_SimpleTest_ScalarValue_Module() {
  const char *fixture = "simple_test/mutators/scalar_value/scalar_value.bc";
  return createModuleFromBitcode(fixture, fixture);
}

#pragma mark - Replace Assignment

std::unique_ptr<MullModule> TestModuleFactory::create_SimpleTest_ReplaceAssignment_Module() {
    const char *fixture = "simple_test/mutators/replace_assignment/replace_assignment.bc";
    return createModuleFromBitcode(fixture, fixture);
}

#pragma mark - Replace Call

std::unique_ptr<MullModule> TestModuleFactory::create_SimpleTest_ReplaceCall_Module() {
  const char *fixture = "simple_test/mutators/replace_call/replace_call.bc";
  return createModuleFromBitcode(fixture, fixture);
}

#pragma mark -

std::string TestModuleFactory::testerModulePath_Bitcode() {
  return fixturePath("fixture_simple_test_tester_module.bc");
}

std::unique_ptr<MullModule> TestModuleFactory::create_SimpleTest_CountLettersTest_Module() {
  const char *fixture = "simple_test/count_letters/test_count_letters.bc";
  return createModuleFromBitcode(fixture, fixture);
}

std::unique_ptr<MullModule> TestModuleFactory::create_SimpleTest_CountLetters_Module() {
  return createModuleFromBitcode("simple_test/count_letters/count_letters.bc", "count_letters");
}

#pragma mark - Google Test

std::unique_ptr<MullModule> TestModuleFactory::create_GoogleTest_Tester_Module() {
  return createModuleFromBitcode("google_test/google_test/Test.bc",
                                 "google_test_tester");
}

std::unique_ptr<MullModule> TestModuleFactory::create_GoogleTest_Testee_Module() {
  return createModuleFromBitcode("google_test/google_test/Testee.bc",
                                 "google_test_testee");
}

#pragma mark -

std::unique_ptr<MullModule> TestModuleFactory::APInt_9a3c2a89c9f30b6c2ab9a1afce2b65d6_213_0_17_negate_mutatorModule() {
  const char *fixture = "APInt_9a3c2a89c9f30b6c2ab9a1afce2b65d6_213_0_17_negate_mutator.ll";
  return createModule(fixture, fixture);
}

std::unique_ptr<MullModule> TestModuleFactory::APFloat_019fc57b8bd190d33389137abbe7145e_214_2_7_negate_mutatorModule() {
  const char *fixture = "APFloat_019fc57b8bd190d33389137abbe7145e_214_2_7_negate_mutator.ll";
  return createModule(fixture, fixture);
}

std::unique_ptr<MullModule> TestModuleFactory::APFloat_019fc57b8bd190d33389137abbe7145e_5_1_3_negate_mutatorModule() {
  const char *fixture = "APFloat_019fc57b8bd190d33389137abbe7145e_5_1_3_negate_mutator.ll";
  return createModule(fixture, fixture);
}

std::unique_ptr<MullModule> TestModuleFactory::create_CustomTest_Distance_Distance_Module() {
  const char *fixture = "custom_test/distance/distance.bc";
  return createModuleFromBitcode(fixture, fixture);
}

std::unique_ptr<MullModule> TestModuleFactory::createCustomTest_Distance_Main_Module() {
  const char *fixture = "custom_test/distance/main.bc";
  return createModuleFromBitcode(fixture, fixture);
}

std::unique_ptr<MullModule> TestModuleFactory::createCustomTest_Distance_Test_Module() {
  const char *fixture = "custom_test/distance/test.bc";
  return createModuleFromBitcode(fixture, fixture);
}

std::unique_ptr<MullModule> TestModuleFactory::createCustomTest_DylibsAndObjects_Test_Module() {
  const char *fixture = "custom_test/dylibs_and_objects/test.bc";
  return createModuleFromBitcode(fixture, fixture);
}

std::unique_ptr<MullModule> TestModuleFactory::createCustomTest_DylibsAndObjects_Main_Module() {
  const char *fixture = "custom_test/dylibs_and_objects/main.bc";
  return createModuleFromBitcode(fixture, fixture);
}

std::unique_ptr<MullModule> TestModuleFactory::createCustomTest_DylibsAndObjects_Distance_Module() {
  const char *fixture = "custom_test/dylibs_and_objects/distance.bc";
  return createModuleFromBitcode(fixture, fixture);
}

std::unique_ptr<MullModule> TestModuleFactory::create_CustomTest_OpenSSL_bio_enc_test_Module() {
  const char *fixture = "openssl_bio_enc_test.o.ll";
  return createModule(fixture, fixture);
}

std::unique_ptr<MullModule> TestModuleFactory::create_ConditionalsBoundaryMutator_Module() {
  const char *fixture = "mutators/boundary/main.bc";
  return createModuleFromBitcode(fixture, fixture);
}

std::unique_ptr<MullModule> TestModuleFactory::create_CompilationDatabase_AbsolutePath_Module() {
  const char *fixture = "junk_detection/compdb_absolute_paths/main.bc";
  return createModuleFromBitcode(fixture, fixture);
}

std::string TestModuleFactory::CompilationDatabase_AbsolutePath_Directory() {
  const char *directory = "junk_detection/compdb_absolute_paths/";
  return fixturePath(directory);
}

std::unique_ptr<MullModule> TestModuleFactory::create_CompilationDatabase_RelativePath_Module() {
  const char *fixture = "junk_detection/compdb_relative_paths/main.bc";
  return createModuleFromBitcode(fixture, fixture);
}

std::string TestModuleFactory::CompilationDatabase_RelativePath_Directory() {
  const char *directory = "junk_detection/compdb_relative_paths/";
  return fixturePath(directory);
}

#pragma mark - Rust

std::unique_ptr<MullModule> TestModuleFactory::rustModule() {
  const char *fixture = "fixture_rust.ll";
  return createModule(fixture, "rust");
}
