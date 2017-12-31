
#include "ModuleLoader.h"
#include "MullModule.h"

#include <string>

using namespace llvm;
using namespace mull;

class TestModuleFactory {

public:
  static std::string fixturePath(const char *fixtureName);

  std::string testerModulePath_Bitcode();
  
  MullModule createModule(const char *fixtureName,
                                           const char *moduleIdentifier);
  
  MullModule create_SimpleTest_CountLettersTest_Module();
  MullModule create_SimpleTest_CountLetters_Module();

  MullModule create_SimpleTest_MathSub_Module();
  MullModule create_SimpleTest_MathMul_Module();
  MullModule create_SimpleTest_MathDiv_Module();

  MullModule create_SimpleTest_NegateCondition_Tester_Module();
  MullModule create_SimpleTest_NegateCondition_Testee_Module();

  MullModule create_SimpleTest_RemoveVoidFunction_Tester_Module();
  MullModule create_SimpleTest_RemoveVoidFunction_Testee_Module();

  MullModule create_SimpleTest_ANDORReplacement_Module();
  MullModule create_SimpleTest_ANDORReplacement_CPPContent_Module();

  MullModule create_SimpleTest_ScalarValue_Module();

  MullModule create_SimpleTest_ReplaceAssignment_Module();
  MullModule create_SimpleTest_ReplaceCall_Module();

  MullModule create_GoogleTest_Tester_Module();
  MullModule create_GoogleTest_Testee_Module();

  MullModule APInt_9a3c2a89c9f30b6c2ab9a1afce2b65d6_213_0_17_negate_mutation_operatorModule();
  MullModule APFloat_019fc57b8bd190d33389137abbe7145e_214_2_7_negate_mutation_operatorModule();
  MullModule APFloat_019fc57b8bd190d33389137abbe7145e_5_1_3_negate_mutation_operatorModule();

  MullModule create_CustomTest_Distance_Distance_Module();
  MullModule createCustomTest_Distance_Main_Module();
  MullModule createCustomTest_Distance_Test_Module();

  MullModule createCustomTest_DylibsAndObjects_Test_Module();
  MullModule createCustomTest_DylibsAndObjects_Main_Module();
  MullModule createCustomTest_DylibsAndObjects_Distance_Module();

  MullModule create_CustomTest_OpenSSL_bio_enc_test_Module();

#pragma mark - Rust
  MullModule rustModule();
};

class FakeModuleLoader : public mull::ModuleLoader {
  std::function<std::vector<MullModule> ()> modules;

public:
  FakeModuleLoader(LLVMContext &context, std::function<std::vector<MullModule> ()> modules) : ModuleLoader(context), modules(modules) {}

  std::vector<MullModule>
  loadModulesFromBitcodeFileList(const std::vector<std::string> &paths) override {
    std::function<std::vector<MullModule> ()> modules = this->modules;

    if (modules) {
      return modules();
    }

    return {};
  }
  
};
