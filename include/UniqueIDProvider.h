#include <string>

namespace llvm {

  class Module;

}

namespace Mutang {
  class MutangModule;
  class MutationPoint;

  class UniqueIDProvider {
  public:
    std::string uniqueIDForModule(const MutangModule &module);
    std::string uniqueIDForModuleWithMutationPoint(const MutangModule &module,
                                                   const MutationPoint &point);
  };
}
