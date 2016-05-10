#include "ConfigParser.h"
#include "Config.h"

#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/YAMLParser.h"

#include "gtest/gtest.h"

using namespace Mutang;
using namespace llvm;

TEST(ConfigParser, loadConfig) {
    SourceMgr SM;
    yaml::Stream Stream("bitcode_files:\n"
                        "  - foo.bc\n"
                        "  - bar.bc\n", SM);

    ConfigParser Parser;
    auto Cfg = Parser.loadConfig(Stream);

    ASSERT_EQ(2U, Cfg->GetBitcodePaths().size());
    ASSERT_EQ("foo.bc", *(Cfg->GetBitcodePaths().begin()));
    ASSERT_EQ("bar.bc", *(Cfg->GetBitcodePaths().end() - 1));
}
