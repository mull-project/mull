#pragma once

#include <string>
#include <vector>

namespace Mutang {

class Config {
    std::vector<std::string> BitcodePaths;
public:
    Config(const std::vector<std::string> &Paths) : BitcodePaths(Paths) {}
    const std::vector<std::string> &GetBitcodePaths() const {
        return BitcodePaths;
    }
};

}
