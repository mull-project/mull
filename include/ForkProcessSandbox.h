
#include <functional>

namespace Mutang {

struct ExecutionResult;

class ForkProcessSandbox {

public:
  ExecutionResult run(std::function<void (ExecutionResult *)> Func);
};

};
