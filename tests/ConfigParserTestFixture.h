#include "mull/Config/ConfigParser.h"

#include "gtest/gtest.h"

using namespace llvm;
using namespace mull;

class ConfigParserTestFixture : public ::testing::Test {
protected:
  RawConfig config;

  void configWithYamlContent(const char *content) {
    yaml::Input Input(content);
    ConfigParser Parser;
    config = Parser.loadConfig(Input);
  }

  void configWithAMinimalValidYamlContent(const char *content) {
    static std::string BitcodeFileList = "/tmp/bitcode_file_list.txt";

    std::ofstream fs(BitcodeFileList);

    if (!fs) {
      std::cerr << "Cannot open the output file." << std::endl;
      FAIL();
    }

    fs << "foo.bc" << std::endl;
    fs << "bar.bc" << std::endl;
    fs << "# baz.bc" << std::endl;

    const char *minimalConfigYAML =
        "bitcode_file_list: /tmp/bitcode_file_list.txt\n";

    std::string fullConfigYaml = minimalConfigYAML;
    fullConfigYaml += content;

    configWithYamlContent(fullConfigYaml.c_str());
  }
};
