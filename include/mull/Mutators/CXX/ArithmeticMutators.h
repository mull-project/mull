#pragma once

#include "TrivialCXXMutator.h"

namespace mull {

namespace cxx {

class AddToSub : public TrivialCXXMutator {
public:
  static std::string ID();
  AddToSub();
};

class AddAssignToSubAssign : public TrivialCXXMutator {
public:
  static std::string ID();
  AddAssignToSubAssign();
};

class PostIncToPostDec : public TrivialCXXMutator {
public:
  static std::string ID();
  PostIncToPostDec();
};

class PreIncToPreDec : public TrivialCXXMutator {
public:
  static std::string ID();
  PreIncToPreDec();
};

class SubToAdd : public TrivialCXXMutator {
public:
  static std::string ID();
  SubToAdd();
};

class SubAssignToAddAssign : public TrivialCXXMutator {
public:
  static std::string ID();
  SubAssignToAddAssign();
};

class PostDecToPostInc : public TrivialCXXMutator {
public:
  static std::string ID();
  PostDecToPostInc();
};

class PreDecToPreInc : public TrivialCXXMutator {
public:
  static std::string ID();
  PreDecToPreInc();
};

class MulToDiv : public TrivialCXXMutator {
public:
  static std::string ID();
  MulToDiv();
};

class MulAssignToDivAssign : public TrivialCXXMutator {
public:
  static std::string ID();
  MulAssignToDivAssign();
};

class DivToMul : public TrivialCXXMutator {
public:
  static std::string ID();
  DivToMul();
};

class DivAssignToMulAssign : public TrivialCXXMutator {
public:
  static std::string ID();
  DivAssignToMulAssign();
};

class RemToDiv : public TrivialCXXMutator {
public:
  static std::string ID();
  RemToDiv();
};

class RemAssignToDivAssign : public TrivialCXXMutator {
public:
  static std::string ID();
  RemAssignToDivAssign();
};

class BitwiseNotToNoop : public TrivialCXXMutator {
public:
  static std::string ID();
  BitwiseNotToNoop();
};

class UnaryMinusToNoop : public Mutator {
public:
  static std::string ID();
  UnaryMinusToNoop();

  std::string getUniqueIdentifier() override;
  std::string getUniqueIdentifier() const override;
  std::string getDescription() const override;
  std::string getDiagnostics() const override;
  std::string getReplacement() const override;
  MutatorKind mutatorKind() override;

  void applyMutation(llvm::Function *function, const MutationPointAddress &address,
                     irm::IRMutation *lowLevelMutation) override;

  std::vector<MutationPoint *> getMutations(Bitcode *bitcode,
                                            const FunctionUnderTest &function) override;

private:
  std::vector<std::unique_ptr<irm::IRMutation>> lowLevelMutators;
};

} // namespace cxx

} // namespace mull