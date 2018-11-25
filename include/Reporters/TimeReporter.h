#include "Reporters/Reporter.h"

namespace mull {

class TimeReporter : public Reporter {
public:
  void reportResults(const Result &result,
                     const RawConfig &config,
                     const Metrics &metrics) override;
};

}

