#include "GoogleTest/GoogleTest_Test.h"

#include "llvm/IR/Function.h"

#include <cxxabi.h>
#include <string>
#include <sstream>

using namespace Mutang;
using namespace std;

static std::string demangle(const char* name) {
  int status = -1;
  std::unique_ptr<char, void(*)(void*)> res { abi::__cxa_demangle(name, NULL, NULL, &status), std::free };

  assert( (status == 0) && "Can't demangle Test Function name");

  return res.get();
}

static vector<string> split_non_empty(string input, char delimiter) {
  stringstream ss(input);
  string item;
  vector<string> elems;
  while (getline(ss, item, delimiter)) {
    if (item.size()) {
      elems.push_back(item);
    }
  }

  return elems;
}

GoogleTest_Test::GoogleTest_Test(llvm::Function *Function,
                                 std::vector<llvm::Function *> Ctors) :
  Test(TK_GoogleTest), TestFunction(Function), GlobalCtors(Ctors)
{
  DetermineSuiteAndTestNames();
}

llvm::Function *GoogleTest_Test::GetTestFunction() {
  return TestFunction;
}

std::string GoogleTest_Test::getTestName() {
  return TestName;
}

std::string GoogleTest_Test::getTestSuiteName() {
  return TestSuiteName;
}

std::vector<llvm::Function *> &GoogleTest_Test::GetGlobalCtors() {
  return GlobalCtors;
}

void GoogleTest_Test::DetermineSuiteAndTestNames() {
  string demangledName = demangle(TestFunction->getName().str().c_str());

  /// Test function has name of the following forms
  ///   some::namespaces::SuiteName_TestName_Test::TestBody()
  ///   SuiteName_TestName_Test::TestBody() // <- no namespaces
  ///
  /// To get names of test and test suite we split string by '::',
  /// strip "TestBody()" and take first element from the end
  /// This way we don't care if namespaces are presented or not
  ///
  /// Once we get SuiteName_TestName we can split it by '_'
  /// and take SuiteName and TestName ignoring last part 'Test'
  ///
  /// FIXME: Performance here is pretty crappy
  /// FIXME: Algorithm is not correct
  /// The algorithm works correctly, if we assume
  /// that test suite and test names themself do not contain underscores
  /// Put assertion there to check how often the assumtion is wrong
  ///
  /// EXCUSE: this is not what I care about at midnight
  auto testMethodChunks = split_non_empty(demangledName, ':');

  assert(testMethodChunks.size() >= 2);

  auto suiteAndTestNames = split_non_empty(*(testMethodChunks.end() - 2), '_');

//  assert((suiteAndTestNames.size() == 3) && "Assumption about names is wrong");

  TestSuiteName = *(suiteAndTestNames.begin());
  TestName = *(suiteAndTestNames.begin() + 1);
}
