
#include <functional>

namespace Mutang {

struct ExecutionResult;

class ProcessSandbox {
public:
  virtual ~ProcessSandbox() {}
  virtual ExecutionResult run(std::function<void (ExecutionResult *)> Func) = 0;
};

class ForkProcessSandbox : public ProcessSandbox {
public:
  ExecutionResult run(std::function<void (ExecutionResult *)> Func);
};

class NullProcessSandbox : public ProcessSandbox {
public:
  ExecutionResult run(std::function<void (ExecutionResult *)> Func);
};

}
