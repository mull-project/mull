#pragma once

#include "Config.h"

namespace llvm {

namespace yaml {

class Stream;

}

}

namespace Mutang {

class ConfigParser {
public:
    std::unique_ptr<Config> loadConfig(llvm::yaml::Stream &S);
};

}
