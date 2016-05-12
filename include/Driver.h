#pragma once

namespace Mutang {

class Config;
class ModuleLoader;

class Driver {
  Config &Cfg;
  ModuleLoader &Loader;
public:
  Driver(Config &C, ModuleLoader &ML) : Cfg(C), Loader(ML) {}
  void Run() {
    /// Populate Mutang::Context with modules using
    /// ModulePaths from Mutang::Config.
    /// Mutang::Context should be populated using ModuleLoader
    /// so that we could inject modules from string for testing purposes

    /// Having Mutang::Context in place we could instantiate TestFinder and find all tests
    /// Using same TestFinder we could find mutation points, apply them sequentially and
    /// run tests/mutants using newly created TestRunner

    /// This method should return (somehow) results of the tests/mutants execution
    /// So that we could easily plug in some TestReporter
  }
};

}
