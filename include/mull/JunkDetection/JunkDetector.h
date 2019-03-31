#pragma once

namespace mull {

class MutationPoint;

class JunkDetector {
public:
  virtual bool isJunk(MutationPoint *point) = 0;
  virtual ~JunkDetector() = default;
};

class NullJunkDetector : public JunkDetector {
public:
  bool isJunk(MutationPoint *point) override { return false; }
};

class AllJunkDetector : public JunkDetector {
public:
  bool isJunk(MutationPoint *point) override { return true; }
};

} // namespace mull
