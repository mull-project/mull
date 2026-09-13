#include <gtest/gtest.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <memory>
#include <mull/Mutators/CXX/IntegerStoreReplacement.h>

using namespace mull;
using namespace llvm;

namespace {

/// An empty function taking one i8, for tests to fill in with the IR they need.
class ModuleFixture {
public:
  ModuleFixture() : module(std::make_unique<Module>("test", context)) {
    FunctionType *functionType =
        FunctionType::get(Type::getVoidTy(context), { Type::getInt8Ty(context) }, false);
    function = Function::Create(functionType, Function::ExternalLinkage, "testee", module.get());
    builder = std::make_unique<IRBuilder<>>(BasicBlock::Create(context, "entry", function));
  }

  LLVMContext &getContext() {
    return context;
  }

  IRBuilder<> &getBuilder() {
    return *builder;
  }

  Argument *getArgument() {
    return function->getArg(0);
  }

  Module *getModule() {
    return module.get();
  }

private:
  LLVMContext context;
  std::unique_ptr<Module> module;
  Function *function = nullptr;
  std::unique_ptr<IRBuilder<>> builder;
};

/// How a stack slot is read is what tells a bool apart from the other
/// byte-sized types: a bool is truncated to i1, a char is not.
enum class Read {
  /// `trunc iN %v to i1` - how clang reads a bool.
  Boolean,
  /// `icmp ne iN %v, 0` - how clang reads a char.
  Integer,
  /// The slot is never read.
  None,
};

void addRead(ModuleFixture &fixture, Value *slot, Type *slotType, Read read) {
  if (read == Read::None) {
    return;
  }
  LoadInst *load = fixture.getBuilder().CreateLoad(slotType, slot);
  if (read == Read::Boolean) {
    fixture.getBuilder().CreateTrunc(load, Type::getInt1Ty(fixture.getContext()));
  } else {
    fixture.getBuilder().CreateICmpNE(load, ConstantInt::get(slotType, 0));
  }
}

/// `T slot = value;` followed by a read of `slot`.
StoreInst *constantStore(ModuleFixture &fixture, unsigned bitWidth, uint64_t value, Read read) {
  Type *slotType = Type::getIntNTy(fixture.getContext(), bitWidth);
  AllocaInst *slot = fixture.getBuilder().CreateAlloca(slotType);
  StoreInst *store = fixture.getBuilder().CreateStore(ConstantInt::get(slotType, value), slot);
  addRead(fixture, slot, slotType, read);
  return store;
}

int64_t storedValue(const StoreInst *store) {
  return cast<ConstantInt>(store->getValueOperand())->getSExtValue();
}

int64_t mutatedValue(StoreInst *store) {
  cxx::IntegerStoreReplacement mutation(42, "{}");
  mutation.mutate(store);
  return storedValue(store);
}

std::optional<bool> negated(const Instruction *instruction) {
  return cxx::IntegerStoreReplacement::negatedBooleanValue(instruction);
}

std::optional<std::string> described(const Instruction *instruction) {
  cxx::IntegerStoreReplacement mutation(42, "{}");
  return mutation.describeReplacement(instruction);
}

} // namespace

TEST(IntegerStoreReplacement, NegatesBooleanFalseToTrue) {
  ModuleFixture fixture;
  StoreInst *store = constantStore(fixture, 8, 0, Read::Boolean);

  ASSERT_TRUE(negated(store).has_value());
  ASSERT_TRUE(*negated(store));
  ASSERT_EQ(mutatedValue(store), 1);
}

TEST(IntegerStoreReplacement, NegatesBooleanTrueToFalse) {
  ModuleFixture fixture;
  StoreInst *store = constantStore(fixture, 8, 1, Read::Boolean);

  ASSERT_TRUE(negated(store).has_value());
  ASSERT_FALSE(*negated(store));
  ASSERT_EQ(mutatedValue(store), 0);
}

/// A char and a bool are the same `store i8 0` in IR. Only the bool is read
/// through a trunc to i1, so only the bool may be negated - negating a char
/// turns a mutant the tests could kill into an equivalent one. Concretely,
/// `signed char a = 0; ... if (a > 5)` is killed by the 42 mutant but not by
/// a 1 mutant.
TEST(IntegerStoreReplacement, KeepsConstantForByteSizedNonBoolean) {
  ModuleFixture fixture;
  StoreInst *store = constantStore(fixture, 8, 0, Read::Integer);

  ASSERT_FALSE(negated(store).has_value());
  ASSERT_EQ(mutatedValue(store), 42);
}

TEST(IntegerStoreReplacement, KeepsConstantForByteSizedValueThatIsNeverRead) {
  ModuleFixture fixture;
  StoreInst *store = constantStore(fixture, 8, 1, Read::None);

  ASSERT_FALSE(negated(store).has_value());
  ASSERT_EQ(mutatedValue(store), 42);
}

/// short, int and long hold 0 and 1 just as a bool does; none of them is read
/// through a trunc to i1, so none of them is negated.
TEST(IntegerStoreReplacement, KeepsConstantForWiderIntegers) {
  for (unsigned bitWidth : { 16u, 32u, 64u }) {
    for (uint64_t value : { 0u, 1u }) {
      ModuleFixture fixture;
      StoreInst *store = constantStore(fixture, bitWidth, value, Read::Integer);

      ASSERT_FALSE(negated(store).has_value()) << "i" << bitWidth << " storing " << value;
      ASSERT_EQ(mutatedValue(store), 42) << "i" << bitWidth << " storing " << value;
    }
  }
}

/// Only 0 and 1 are the bool representation; any other constant is a plain
/// integer even when the slot is read as a bool.
TEST(IntegerStoreReplacement, KeepsConstantForNonBooleanConstant) {
  ModuleFixture fixture;
  StoreInst *store = constantStore(fixture, 8, 7, Read::Boolean);

  ASSERT_FALSE(negated(store).has_value());
  ASSERT_EQ(mutatedValue(store), 42);
}

/// The Itanium static-initialization guard variable is a byte holding 0 or 1
/// that is compared with `icmp eq`, never truncated. Negating its `store i8 1`
/// would leave the guard at 0, re-running a static local's constructor on every
/// call - for a `static std::mutex` or `static std::ofstream` that corrupts or
/// hangs the test process instead of failing cleanly.
TEST(IntegerStoreReplacement, KeepsConstantForStaticInitializationGuard) {
  ModuleFixture fixture;
  Type *guardType = Type::getInt8Ty(fixture.getContext());
  auto *guard = new GlobalVariable(*fixture.getModule(),
                                   guardType,
                                   false,
                                   GlobalValue::InternalLinkage,
                                   ConstantInt::get(guardType, 0),
                                   "_ZGVZ1fvE4lazy");

  LoadInst *load = fixture.getBuilder().CreateLoad(guardType, guard);
  fixture.getBuilder().CreateICmpEQ(load, ConstantInt::get(guardType, 0));
  StoreInst *store = fixture.getBuilder().CreateStore(ConstantInt::get(guardType, 1), guard);

  ASSERT_FALSE(negated(store).has_value());
  ASSERT_EQ(mutatedValue(store), 42);
}

/// A bool assigned a runtime value has no constant to negate, so it keeps the
/// 42 replacement. Classifying it by the slot alone would report a boolean
/// mutant for a store that is not one.
TEST(IntegerStoreReplacement, KeepsReplacementForNonConstantBooleanStore) {
  ModuleFixture fixture;
  Type *slotType = Type::getInt8Ty(fixture.getContext());
  AllocaInst *slot = fixture.getBuilder().CreateAlloca(slotType);
  StoreInst *store = fixture.getBuilder().CreateStore(fixture.getArgument(), slot);
  addRead(fixture, slot, slotType, Read::Boolean);

  ASSERT_FALSE(negated(store).has_value());

  cxx::IntegerStoreReplacement mutation(42, "{}");
  ASSERT_TRUE(mutation.canMutate(store));
  mutation.mutate(store);
  ASSERT_EQ(storedValue(store), 42);
}

/// A vector store is not an integer store: classifying it by the constant's
/// bit width would ask a vector type for its integer width.
TEST(IntegerStoreReplacement, IgnoresVectorStores) {
  ModuleFixture fixture;
  Type *vectorType = FixedVectorType::get(Type::getInt8Ty(fixture.getContext()), 4);
  AllocaInst *slot = fixture.getBuilder().CreateAlloca(vectorType);
  StoreInst *store = fixture.getBuilder().CreateStore(Constant::getNullValue(vectorType), slot);

  cxx::IntegerStoreReplacement mutation(42, "{}");
  ASSERT_FALSE(mutation.canMutate(store));
  ASSERT_FALSE(negated(store).has_value());
}

/// negatedBooleanValue is public and answers for any instruction, so it has to
/// hold up on non-stores and on null rather than rely on an assert that is
/// compiled out of a release build.
TEST(IntegerStoreReplacement, IgnoresInstructionsThatAreNotStores) {
  ModuleFixture fixture;
  Type *slotType = Type::getInt8Ty(fixture.getContext());
  AllocaInst *slot = fixture.getBuilder().CreateAlloca(slotType);
  LoadInst *load = fixture.getBuilder().CreateLoad(slotType, slot);

  cxx::IntegerStoreReplacement mutation(42, "{}");
  ASSERT_FALSE(mutation.canMutate(load));
  ASSERT_FALSE(mutation.canMutate(slot));
  ASSERT_FALSE(negated(load).has_value());
  ASSERT_FALSE(negated(nullptr).has_value());

  /// Mutating an instruction it cannot mutate leaves the IR alone.
  mutation.mutate(load);
  ASSERT_TRUE(isa<LoadInst>(load));
}

/// A negated boolean store reports the constant it actually stores; naming the
/// mutator's 42 there would describe a value the mutant never holds.
TEST(IntegerStoreReplacement, DescribesTheNegatedConstant) {
  ModuleFixture falseFixture;
  ASSERT_EQ(described(constantStore(falseFixture, 8, 0, Read::Boolean)), "1");

  ModuleFixture trueFixture;
  ASSERT_EQ(described(constantStore(trueFixture, 8, 1, Read::Boolean)), "0");
}

/// A store it does not negate is described with the mutator's own constant, so
/// the reported text always names the value the mutant holds.
TEST(IntegerStoreReplacement, DescribesTheMutatorConstantForStoresItDoesNotNegate) {
  ModuleFixture charFixture;
  ASSERT_EQ(described(constantStore(charFixture, 8, 0, Read::Integer)), "42");

  ModuleFixture intFixture;
  ASSERT_EQ(described(constantStore(intFixture, 32, 1, Read::Integer)), "42");
}

/// Only integer stores are described at all; anything else falls back to the
/// mutator's replacement.
TEST(IntegerStoreReplacement, DescribesNothingForInstructionsItCannotMutate) {
  ModuleFixture fixture;
  Type *slotType = Type::getInt8Ty(fixture.getContext());
  AllocaInst *slot = fixture.getBuilder().CreateAlloca(slotType);
  LoadInst *load = fixture.getBuilder().CreateLoad(slotType, slot);

  ASSERT_FALSE(described(load).has_value());
  ASSERT_FALSE(described(nullptr).has_value());
}

/// The spelling carries the affixes, so the same mutation renders the value the
/// way its mutator words it.
TEST(IntegerStoreReplacement, RendersTheValueIntoTheSpelling) {
  ModuleFixture fixture;
  StoreInst *store = constantStore(fixture, 8, 1, Read::Boolean);

  cxx::IntegerStoreReplacement assign(42, "= {};");
  ASSERT_EQ(assign.describeReplacement(store), "= 0;");
}
