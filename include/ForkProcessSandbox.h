
#include <functional>

namespace mull {

struct ExecutionResult;

class ProcessSandbox {
public:
  virtual ~ProcessSandbox() {}
  virtual ExecutionResult run(std::function<void (ExecutionResult *)> function,
                              long long timeoutMilliseconds) = 0;
};

class ForkProcessSandbox : public ProcessSandbox {
public:
  const static int MullExitCode = 227;
  const static int MullTimeoutCode = 239;

  ExecutionResult run(std::function<void (ExecutionResult *)> function,
                      long long timeoutMilliseconds);
};

class NullProcessSandbox : public ProcessSandbox {
public:
  ExecutionResult run(std::function<void (ExecutionResult *)> function,
                      long long timeoutMilliseconds);
};

}
