#pragma once

namespace Mutang {

class Config;

class Driver {
    Config &Cfg;
public:
    Driver(Config &C) : Cfg(C) {}
};

}
