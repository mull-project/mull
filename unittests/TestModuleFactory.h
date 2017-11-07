
#include "ModuleLoader.h"
#include "MullModule.h"

#include <string>

using namespace llvm;
using namespace mull;

class TestModuleFactory {

public:
  std::string testerModulePath_IR();
  std::string testerModulePath_Bitcode();
  
  std::unique_ptr<MullModule> createModule(const char *fixtureName,
                                           const char *moduleIdentifier);
  
  std::unique_ptr<MullModule> createTesterModule();
  std::unique_ptr<MullModule> createTesteeModule();

  std::unique_ptr<MullModule> create_SimpleTest_MathSub_module();
  std::unique_ptr<MullModule> create_SimpleTest_MathMul_module();
  std::unique_ptr<MullModule> create_SimpleTest_MathDiv_module();

  std::unique_ptr<MullModule> create_SimpleTest_NegateCondition_Tester_Module();
  std::unique_ptr<MullModule> create_SimpleTest_NegateCondition_Testee_Module();

  std::unique_ptr<MullModule> create_SimpleTest_RemoveVoidFunction_Tester_Module();
  std::unique_ptr<MullModule> create_SimpleTest_RemoveVoidFunction_Testee_Module();

  std::unique_ptr<MullModule> create_SimpleTest_ANDORReplacement_Module();
  std::unique_ptr<MullModule> create_SimpleTest_ANDORReplacement_CPPContent_Module();

  std::unique_ptr<MullModule> create_SimpleTest_ScalarValue_module();

  std::unique_ptr<MullModule> create_SimpleTest_ReplaceCall_module();

  std::unique_ptr<MullModule> create_SimpleTest_testeePathCalculation_testee();
  std::unique_ptr<MullModule> create_SimpleTest_testeePathCalculation_tester();

  std::unique_ptr<MullModule> createLibCTesterModule();
  std::unique_ptr<MullModule> createLibCTesteeModule();

  std::unique_ptr<MullModule> createExternalLibTesterModule();
  std::unique_ptr<MullModule> createExternalLibTesteeModule();

  std::unique_ptr<MullModule> createGoogleTestTesterModule();
  std::unique_ptr<MullModule> createGoogleTestTesteeModule();
  std::unique_ptr<MullModule> createGoogleTestFinder_invokeInstTestee_Module();

  std::unique_ptr<MullModule> APInt_9a3c2a89c9f30b6c2ab9a1afce2b65d6_213_0_17_negate_mutation_operatorModule();
  std::unique_ptr<MullModule> APFloat_019fc57b8bd190d33389137abbe7145e_214_2_7_negate_mutation_operatorModule();
  std::unique_ptr<MullModule> APFloat_019fc57b8bd190d33389137abbe7145e_5_1_3_negate_mutation_operatorModule();

  std::unique_ptr<MullModule> createCustomTest_Distance_DistanceModule();
  std::unique_ptr<MullModule> createCustomTest_Distance_MainModule();
  std::unique_ptr<MullModule> createCustomTest_Distance_TestModule();

  std::unique_ptr<MullModule> create_CustomTest_OpenSSL_bio_enc_test_module();

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
