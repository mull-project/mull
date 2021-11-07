#include "mull/Mutators/CXX/ArithmeticMutators.h"
#include "mull/MutationPoint.h"

using namespace mull;
using namespace mull::cxx;

/// All add to sub mutators share the same set of low level mutators
static std::vector<std::unique_ptr<irm::IRMutation>> getAddToSub() {
  std::vector<std::unique_ptr<irm::IRMutation>> mutators;
  mutators.emplace_back(new irm::AddToSub());
  mutators.emplace_back(new irm::FAddToFSub());
  mutators.emplace_back(new irm::sadd_with_overflowTossub_with_overflow());
  return mutators;
}

std::string AddToSub::ID() {
  return "cxx_add_to_sub";
}

AddToSub::AddToSub()
    : TrivialCXXMutator(getAddToSub(), MutatorKind::CXX_AddToSub, AddToSub::ID(),
                        "Replaces + with -", "-", "Replaced + with -") {}

std::string AddAssignToSubAssign::ID() {
  return "cxx_add_assign_to_sub_assign";
}

AddAssignToSubAssign::AddAssignToSubAssign()
    : TrivialCXXMutator(getAddToSub(), MutatorKind::CXX_AddAssignToSubAssign,
                        AddAssignToSubAssign::ID(),
                        "Replaces += with -=", "-=", "Replaced += with -=") {}

std::string PostIncToPostDec::ID() {
  return "cxx_post_inc_to_post_dec";
}

PostIncToPostDec::PostIncToPostDec()
    : TrivialCXXMutator(getAddToSub(), MutatorKind::CXX_PostIncToPostDec, PostIncToPostDec::ID(),
                        "Replaces x++ with x--", "--", "Replaced x++ with x--") {}

std::string PreIncToPreDec::ID() {
  return "cxx_pre_inc_to_pre_dec";
}

PreIncToPreDec::PreIncToPreDec()
    : TrivialCXXMutator(getAddToSub(), MutatorKind::CXX_PreIncToPreDec, PreIncToPreDec::ID(),
                        "Replaces ++x with --x", "--", "Replaced ++x with --x") {}

static std::vector<std::unique_ptr<irm::IRMutation>> getSubToAdd() {
  std::vector<std::unique_ptr<irm::IRMutation>> mutators;
  mutators.emplace_back(new irm::SubToAdd());
  mutators.emplace_back(new irm::FSubToFAdd());
  mutators.emplace_back(new irm::ssub_with_overflowTosadd_with_overflow());
  return mutators;
}

std::string SubToAdd::ID() {
  return "cxx_sub_to_add";
}

SubToAdd::SubToAdd()
    : TrivialCXXMutator(getSubToAdd(), MutatorKind::CXX_SubToAdd, SubToAdd::ID(),
                        "Replaces - with +", "+", "Replaced - with +") {}

std::string SubAssignToAddAssign::ID() {
  return "cxx_sub_assign_to_add_assign";
}

SubAssignToAddAssign::SubAssignToAddAssign()
    : TrivialCXXMutator(getSubToAdd(), MutatorKind::CXX_SubAssignToAddAssign,
                        SubAssignToAddAssign::ID(),
                        "Replaces -= with +=", "+=", "Replaced -= with +=") {}

static std::vector<std::unique_ptr<irm::IRMutation>> getDecToInc() {
  std::vector<std::unique_ptr<irm::IRMutation>> mutators;
  mutators.emplace_back(new irm::SubToAdd());
  mutators.emplace_back(new irm::FSubToFAdd());
  mutators.emplace_back(new irm::ssub_with_overflowTosadd_with_overflow());

  /// This is somewhat non-trivial:
  /// pre and post decrements lowered to IR as
  ///     add x, -1
  /// instead of
  ///     sub x, 1
  /// So we have to consider add instructions as the one producing sub-to-add
  /// mutations
  mutators.emplace_back(new irm::AddToSub());
  return mutators;
}

std::string PostDecToPostInc::ID() {
  return "cxx_post_dec_to_post_inc";
}

PostDecToPostInc::PostDecToPostInc()
    : TrivialCXXMutator(getDecToInc(), MutatorKind::CXX_PostDecToPostInc, PostDecToPostInc::ID(),
                        "Replaces x-- with x++", "++", "Replaced x-- with x++") {}

std::string PreDecToPreInc::ID() {
  return "cxx_pre_dec_to_pre_inc";
}

PreDecToPreInc::PreDecToPreInc()
    : TrivialCXXMutator(getDecToInc(), MutatorKind::CXX_PreDecToPreInc, PreDecToPreInc::ID(),
                        "Replaces --x with ++x", "++", "Replaced --x with ++x") {}

static std::vector<std::unique_ptr<irm::IRMutation>> getMulToDiv() {
  std::vector<std::unique_ptr<irm::IRMutation>> mutators;
  mutators.emplace_back(new irm::MulToSDiv());
  mutators.emplace_back(new irm::FMulToFDiv());
  return mutators;
}

std::string MulToDiv::ID() {
  return "cxx_mul_to_div";
}

MulToDiv::MulToDiv()
    : TrivialCXXMutator(getMulToDiv(), MutatorKind::CXX_MulToDiv, MulToDiv::ID(),
                        "Replaces * with /", "/", "Replaced * with /") {}

std::string MulAssignToDivAssign::ID() {
  return "cxx_mul_assign_to_div_assign";
}

MulAssignToDivAssign::MulAssignToDivAssign()
    : TrivialCXXMutator(getMulToDiv(), MutatorKind::CXX_MulAssignToDivAssign,
                        MulAssignToDivAssign::ID(),
                        "Replaces *= with /=", "/=", "Replaced *= with /=") {}

static std::vector<std::unique_ptr<irm::IRMutation>> getDivToMul() {
  std::vector<std::unique_ptr<irm::IRMutation>> mutators;
  mutators.emplace_back(new irm::FDivToFMul());
  mutators.emplace_back(new irm::SDivToMul());
  mutators.emplace_back(new irm::UDivToMul());
  return mutators;
}

std::string DivToMul::ID() {
  return "cxx_div_to_mul";
}

DivToMul::DivToMul()
    : TrivialCXXMutator(getDivToMul(), MutatorKind::CXX_DivToMul, DivToMul::ID(),
                        "Replaces / with *", "*", "Replaced / with *") {}

std::string DivAssignToMulAssign::ID() {
  return "cxx_div_assign_to_mul_assign";
}

DivAssignToMulAssign::DivAssignToMulAssign()
    : TrivialCXXMutator(getDivToMul(), MutatorKind::CXX_DivAssignToMulAssign,
                        DivAssignToMulAssign::ID(),
                        "Replaces /= with *=", "*=", "Replaced /= with *=") {}

static std::vector<std::unique_ptr<irm::IRMutation>> getRemToDiv() {
  std::vector<std::unique_ptr<irm::IRMutation>> mutators;
  mutators.emplace_back(new irm::FRemToFDiv());
  mutators.emplace_back(new irm::SRemToSDiv());
  mutators.emplace_back(new irm::URemToUDiv());
  return mutators;
}

std::string RemToDiv::ID() {
  return "cxx_rem_to_div";
}

RemToDiv::RemToDiv()
    : TrivialCXXMutator(getRemToDiv(), MutatorKind::CXX_RemToDiv, RemToDiv::ID(),
                        "Replaces % with /", "/", "Replaced % with /") {}

std::string RemAssignToDivAssign::ID() {
  return "cxx_rem_assign_to_div_assign";
}

RemAssignToDivAssign::RemAssignToDivAssign()
    : TrivialCXXMutator(getRemToDiv(), MutatorKind::CXX_RemAssignToDivAssign,
                        RemAssignToDivAssign::ID(),
                        "Replaces %= with /=", "/=", "Replaced %= with /=") {}

static std::vector<std::unique_ptr<irm::IRMutation>> getBitNotToNoop() {
  std::vector<std::unique_ptr<irm::IRMutation>> mutators;
  mutators.emplace_back(new irm::XorToAnd);
  return mutators;
}

std::string BitwiseNotToNoop::ID() {
  return "cxx_bitwise_not_to_noop";
}

BitwiseNotToNoop::BitwiseNotToNoop()
    : TrivialCXXMutator(getBitNotToNoop(), MutatorKind::CXX_BitwiseNotToNoop,
                        BitwiseNotToNoop::ID(), "Replaces ~x with x", "", "Replaced ~x with x") {}

/// Non-trivial mutators

std::string UnaryMinusToNoop::ID() {
  return "cxx_minus_to_noop";
}

UnaryMinusToNoop::UnaryMinusToNoop() {
  lowLevelMutators.emplace_back(new irm::SwapSubOperands);
  lowLevelMutators.emplace_back(new irm::SwapFSubOperands);
#if LLVM_VERSION_MAJOR > 7
  lowLevelMutators.emplace_back(new irm::SwapFNegWithOperand);
#endif
}

std::string UnaryMinusToNoop::getUniqueIdentifier() {
  return UnaryMinusToNoop::ID();
}
std::string UnaryMinusToNoop::getUniqueIdentifier() const {
  return UnaryMinusToNoop::ID();
}

std::string UnaryMinusToNoop::getDescription() const {
  return "Replaces -x with x";
}

std::string UnaryMinusToNoop::getDiagnostics() const {
  return "Replaced -x with x";
}

std::string UnaryMinusToNoop::getReplacement() const {
  return "";
}

MutatorKind UnaryMinusToNoop::mutatorKind() {
  return MutatorKind::CXX_UnaryMinusToNoop;
}

void UnaryMinusToNoop::applyMutation(llvm::Function *function, const MutationPointAddress &address,
                                     irm::IRMutation *lowLevelMutation) {
  llvm::Instruction &instruction = address.findInstruction(function);
  lowLevelMutation->mutate(&instruction);
}

static bool isZero(llvm::Value *value) {
  if (auto constInt = llvm::dyn_cast<llvm::ConstantInt>(value)) {
    return constInt->isZero();
  }
  if (auto constFP = llvm::dyn_cast<llvm::ConstantFP>(value)) {
    return constFP->isZero();
  }

  return false;
}

std::vector<MutationPoint *> UnaryMinusToNoop::getMutations(Bitcode *bitcode,
                                                            const FunctionUnderTest &function) {
  assert(bitcode);

  std::vector<MutationPoint *> mutations;

  for (llvm::Instruction *instruction : function.getSelectedInstructions()) {
    for (auto &mutator : lowLevelMutators) {
      if (mutator->canMutate(instruction)) {
        if (!instruction->isBinaryOp() ||
            (instruction->isBinaryOp() && isZero(instruction->getOperand(0)))) {
          auto point = new MutationPoint(this, mutator.get(), instruction, bitcode);
          mutations.push_back(point);
        }
      }
    }
  }

  return mutations;
}
