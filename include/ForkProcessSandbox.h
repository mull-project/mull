
#include <functional>

namespace Mutang {

struct ExecutionResult;

const long long kDefaultTimeoutInMilliseconds = 3000;

class ProcessSandbox {
public:
  virtual ~ProcessSandbox() {}
  virtual ExecutionResult run(std::function<void (ExecutionResult *)> function,
                              long long timeoutMilliseconds = kDefaultTimeoutInMilliseconds) = 0;
};

class ForkProcessSandbox : public ProcessSandbox {
public:
  ExecutionResult run(std::function<void (ExecutionResult *)> function,
                      long long timeoutMilliseconds = kDefaultTimeoutInMilliseconds);
};

class NullProcessSandbox : public ProcessSandbox {
public:
  ExecutionResult run(std::function<void (ExecutionResult *)> function,
                      long long timeoutMilliseconds = kDefaultTimeoutInMilliseconds);
};

}
