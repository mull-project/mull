
#include "ModuleLoader.h"
#include "MullModule.h"

#include <string>

using namespace llvm;
using namespace mull;

class TestModuleFactory {

public:
  static std::string fixturePath(const char *fixtureName);

  std::string testerModulePath_Bitcode();
  
  std::unique_ptr<MullModule> createModule(const char *fixtureName,
                                           const char *moduleIdentifier);
  
  std::unique_ptr<MullModule> create_SimpleTest_CountLettersTest_Module();
  std::unique_ptr<MullModule> create_SimpleTest_CountLetters_Module();

  std::unique_ptr<MullModule> create_SimpleTest_MathSub_Module();
  std::unique_ptr<MullModule> create_SimpleTest_MathMul_Module();
  std::unique_ptr<MullModule> create_SimpleTest_MathDiv_Module();

  std::unique_ptr<MullModule> create_SimpleTest_NegateCondition_Tester_Module();
  std::unique_ptr<MullModule> create_SimpleTest_NegateCondition_Testee_Module();

  std::unique_ptr<MullModule> create_SimpleTest_RemoveVoidFunction_Tester_Module();
  std::unique_ptr<MullModule> create_SimpleTest_RemoveVoidFunction_Testee_Module();

  std::unique_ptr<MullModule> create_SimpleTest_ANDORReplacement_Module();
  std::unique_ptr<MullModule> create_SimpleTest_ANDORReplacement_CPPContent_Module();

  std::unique_ptr<MullModule> create_SimpleTest_ScalarValue_Module();

  std::unique_ptr<MullModule> create_SimpleTest_ReplaceAssignment_Module();
  std::unique_ptr<MullModule> create_SimpleTest_ReplaceCall_Module();

  std::unique_ptr<MullModule> create_GoogleTest_Tester_Module();
  std::unique_ptr<MullModule> create_GoogleTest_Testee_Module();

  std::unique_ptr<MullModule> APInt_9a3c2a89c9f30b6c2ab9a1afce2b65d6_213_0_17_negate_mutatorModule();
  std::unique_ptr<MullModule> APFloat_019fc57b8bd190d33389137abbe7145e_214_2_7_negate_mutatorModule();
  std::unique_ptr<MullModule> APFloat_019fc57b8bd190d33389137abbe7145e_5_1_3_negate_mutatorModule();

  std::unique_ptr<MullModule> create_CustomTest_Distance_Distance_Module();
  std::unique_ptr<MullModule> createCustomTest_Distance_Main_Module();
  std::unique_ptr<MullModule> createCustomTest_Distance_Test_Module();

  std::unique_ptr<MullModule> createCustomTest_DylibsAndObjects_Test_Module();
  std::unique_ptr<MullModule> createCustomTest_DylibsAndObjects_Main_Module();
  std::unique_ptr<MullModule> createCustomTest_DylibsAndObjects_Distance_Module();

  std::unique_ptr<MullModule> create_CustomTest_OpenSSL_bio_enc_test_Module();

  std::unique_ptr<MullModule> create_ConditionalsBoundaryMutator_Module();
  std::unique_ptr<MullModule> create_CompilationDatabase_AbsolutePath_Module();
  std::string CompilationDatabase_AbsolutePath_Directory();

  std::unique_ptr<MullModule> create_CompilationDatabase_RelativePath_Module();
  std::string CompilationDatabase_RelativePath_Directory();

#pragma mark - Rust
  std::unique_ptr<MullModule> rustModule();

};

class FakeModuleLoader : public mull::ModuleLoader {
  std::function<std::vector<std::unique_ptr<MullModule>> ()> modules;

public:
  FakeModuleLoader(LLVMContext &context, std::function<std::vector<std::unique_ptr<MullModule>> ()> modules) : ModuleLoader(context), modules(modules) {}

  std::vector<std::unique_ptr<MullModule>>
  loadModulesFromBitcodeFileList(const std::vector<std::string> &paths) override {
    std::function<std::vector<std::unique_ptr<MullModule>> ()> modules = this->modules;

    if (modules) {
      return modules();
    }

    return {};
  }
  
};
