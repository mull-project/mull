#include "mull/TestFrameworks/CppUTest/CppUTestFinder.h"

#include "mull/Program/Program.h"

#include <llvm/IR/CallSite.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DebugInfoMetadata.h>
#include <llvm/IR/InstIterator.h>
#include <llvm/IR/Instructions.h>
#include <llvm/Support/raw_ostream.h>

#include <vector>
#include <iostream>

using namespace mull;
using namespace llvm;

std::vector<Test> CppUTestFinder::findTests(Program &program) {
  std::vector<Test> tests;

  auto installerStr = StringRef("class.TestInstaller");

  for (auto &bitcode : program.bitcode()) {
    for (auto &globalValue : bitcode->getModule()->getGlobalList()) {
      Type *globalValueType = globalValue.getValueType();
      if (globalValueType->getTypeID() != Type::StructTyID) {
        continue;
      }

      auto *structType = dyn_cast<StructType>(globalValueType);
      if (!structType) {
        continue;
      }

      if (!structType->getName().startswith(installerStr)) {
        continue;
      }

      CallInst *callInstruction = nullptr;
      for (auto userIterator = globalValue.user_begin();
           userIterator != globalValue.user_end(); userIterator++) {
        auto user = *userIterator;
        if (isa<CallInst>(user)) {
          callInstruction = dyn_cast<CallInst>(user);
          break;
        }
      }

      if(!callInstruction)
        continue;

      auto callSite = CallSite(callInstruction);
      assert((callSite.isCall() || callSite.isInvoke()) &&
             "CallSite should be a call");

      auto testGroupNameConstRef =
          dyn_cast<ConstantExpr>(callSite->getOperand(2));
      assert(testGroupNameConstRef);

      auto testCaseNameConstRef =
          dyn_cast<ConstantExpr>(callSite->getOperand(3));
      assert(testCaseNameConstRef);

      auto testGroupNameConst =
          dyn_cast<GlobalValue>(testGroupNameConstRef->getOperand(0));
      assert(testGroupNameConst);

      auto testCaseNameConst =
          dyn_cast<GlobalValue>(testCaseNameConstRef->getOperand(0));
      assert(testCaseNameConst);

      auto testGroupNameConstArray =
          dyn_cast<ConstantDataArray>(testGroupNameConst->getOperand(0));
      assert(testGroupNameConstArray);

      auto testCaseNameConstArray =
          dyn_cast<ConstantDataArray>(testCaseNameConst->getOperand(0));
      assert(testCaseNameConstArray);

      std::string testGroupName =
          testGroupNameConstArray->getRawDataValues().rtrim('\0').str();
      std::string testCaseName =
          testCaseNameConstArray->getRawDataValues().rtrim('\0').str();

      std::string testBodyFunctionName =
          testGroupName + "_" + testCaseName + "_Test8testBodyEv";
      StringRef testBodyFunctionNameRef(testBodyFunctionName);

      Function *testBodyFunction = nullptr;
      for (auto &func : bitcode->getModule()->getFunctionList()) {
        auto foundPosition = func.getName().rfind(testBodyFunctionNameRef);
        if (foundPosition != StringRef::npos) {
          testBodyFunction = &func;
          break;
        }
      }

      assert(testBodyFunction &&
             "Cannot find the TestBody function for the Test");

      auto arguments = {std::string("-sg"), testGroupName, std::string("-sn"), testCaseName};
      tests.push_back(
          Test(testGroupName + "." + testCaseName, "mull", "main", arguments, testBodyFunction));
    }
  }
  return tests;
}
