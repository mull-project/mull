#pragma once

namespace Mutang {

class Config;
class ModuleLoader;

class Driver {
  Config &Cfg;
  ModuleLoader &Loader;
public:
  Driver(Config &C, ModuleLoader &ML) : Cfg(C), Loader(ML) {}
  void Run();
};

}
