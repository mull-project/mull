#include "Driver.h"
#include "Config.h"

#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/Twine.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/YAMLParser.h"

#include "gtest/gtest.h"

using namespace Mutang;
using namespace llvm;

TEST(Driver, ___) {
    /// Create Config with fake BitcodePaths
    /// Create Fake Module Loader
    /// Initialize Driver using ModuleLoader and Config
    /// Driver should initialize (make them injectable? DI?)
    /// TestRunner and TestFinder based on the Config
    /// Then Run all the tests using driver
}
