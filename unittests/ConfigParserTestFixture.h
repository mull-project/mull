#include "ConfigParser.h"

#include "gtest/gtest.h"

using namespace llvm;
using namespace mull;

class ConfigParserTestFixture : public ::testing::Test {
protected:
  
  Config config;
  
  void configWithYamlContent(const char *content) {
    yaml::Input Input(content);
    ConfigParser Parser;
    config = Parser.loadConfig(Input);
  }
  
};
