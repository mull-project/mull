#include "mull/Diagnostics/Diagnostics.h"
#include "mull/Mutators/MutatorsFactory.h"

#include <functional>
#include <gtest/gtest.h>
#include <vector>

using namespace mull;
using namespace llvm;
using namespace std;

TEST(MutatorsFactory, SingleMutators) {
  Diagnostics diagnostics;
  MutatorsFactory factory(diagnostics);
  vector<unique_ptr<Mutator>> mutators;
  Mutator *mutator = nullptr;

  {
    mutators = factory.mutators({ "negate_mutator" });
    ASSERT_EQ(mutators.size(), 1UL);
    mutator = mutators[0].get();
    ASSERT_EQ(mutator->getUniqueIdentifier(), "negate_mutator");
  }

  {
    mutators = factory.mutators({ "remove_void_function_mutator" });
    ASSERT_EQ(mutators.size(), 1UL);
    mutator = mutators[0].get();
    ASSERT_EQ(mutator->getUniqueIdentifier(), "remove_void_function_mutator");
  }

  {
    mutators = factory.mutators({ "replace_call_mutator" });
    ASSERT_EQ(mutators.size(), 1UL);
    mutator = mutators[0].get();
    ASSERT_EQ(mutator->getUniqueIdentifier(), "replace_call_mutator");
  }

  {
    mutators = factory.mutators({ "scalar_value_mutator" });
    ASSERT_EQ(mutators.size(), 1UL);
    mutator = mutators[0].get();
    ASSERT_EQ(mutator->getUniqueIdentifier(), "scalar_value_mutator");
  }
}

static std::function<bool(unique_ptr<Mutator> &)> predicate(const char *name) {
  return [=](const unique_ptr<Mutator> &o) { return o->getUniqueIdentifier() == name; };
}

TEST(MutatorsFactory, CompositeMutators) {
  Diagnostics diagnostics;
  MutatorsFactory factory(diagnostics);
  vector<unique_ptr<Mutator>> mutators;
  vector<unique_ptr<Mutator>>::iterator searchResult;

  {
    mutators = factory.mutators({ "cxx_arithmetic" });
    ASSERT_EQ(mutators.size(), 6UL);

    searchResult = find_if(mutators.begin(), mutators.end(), predicate("cxx_div_to_mul"));
    ASSERT_NE(searchResult, mutators.end());
    searchResult = find_if(mutators.begin(), mutators.end(), predicate("cxx_mul_to_div"));
    ASSERT_NE(searchResult, mutators.end());
    searchResult = find_if(mutators.begin(), mutators.end(), predicate("cxx_sub_to_add"));
    ASSERT_NE(searchResult, mutators.end());
  }

  {
    mutators = factory.mutators({ "experimental" });
    ASSERT_EQ(mutators.size(), 6UL);

    searchResult = find_if(mutators.begin(), mutators.end(), predicate("replace_call_mutator"));
    ASSERT_NE(searchResult, mutators.end());
    searchResult = find_if(mutators.begin(), mutators.end(), predicate("scalar_value_mutator"));
    ASSERT_NE(searchResult, mutators.end());
  }

  {
    mutators = factory.mutators({ "all" });
    ASSERT_EQ(mutators.size(), 38UL);
  }
}

TEST(MutatorsFactory, UniqueMutators) {
  Diagnostics diagnostics;
  MutatorsFactory factory(diagnostics);
  vector<unique_ptr<Mutator>> mutators;
  vector<unique_ptr<Mutator>>::iterator searchResult;
  Mutator *mutator = nullptr;

  {
    mutators = factory.mutators({ "cxx_add_to_sub", "cxx_add_to_sub" });
    ASSERT_EQ(mutators.size(), 1UL);
    mutator = mutators[0].get();
    ASSERT_EQ(mutator->getUniqueIdentifier(), "cxx_add_to_sub");
  }

  {
    mutators = factory.mutators({ "cxx_arithmetic", "cxx_add_to_sub" });
    ASSERT_EQ(mutators.size(), 6UL);

    searchResult = find_if(mutators.begin(), mutators.end(), predicate("cxx_add_to_sub"));
    ASSERT_NE(searchResult, mutators.end());
    searchResult = find_if(mutators.begin(), mutators.end(), predicate("cxx_div_to_mul"));
    ASSERT_NE(searchResult, mutators.end());
    searchResult = find_if(mutators.begin(), mutators.end(), predicate("cxx_mul_to_div"));
    ASSERT_NE(searchResult, mutators.end());
    searchResult = find_if(mutators.begin(), mutators.end(), predicate("cxx_sub_to_add"));
    ASSERT_NE(searchResult, mutators.end());
  }
}
