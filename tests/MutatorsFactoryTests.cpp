#include "mull/Mutators/MutatorsFactory.h"

#include "gtest/gtest.h"

#include <functional>
#include <vector>

using namespace mull;
using namespace llvm;
using namespace std;

TEST(MutatorsFactory, SingleMutators) {
  MutatorsFactory factory;
  vector<unique_ptr<Mutator>> mutators;
  Mutator *mutator = nullptr;

  {
    mutators = factory.mutators({"math_add_mutator"});
    ASSERT_EQ(mutators.size(), 1UL);
    mutator = mutators[0].get();
    ASSERT_EQ(mutator->getUniqueIdentifier(), "math_add_mutator");
  }

  {
    mutators = factory.mutators({"math_div_mutator"});
    ASSERT_EQ(mutators.size(), 1UL);
    mutator = mutators[0].get();
    ASSERT_EQ(mutator->getUniqueIdentifier(), "math_div_mutator");
  }

  {
    mutators = factory.mutators({"math_mul_mutator"});
    ASSERT_EQ(mutators.size(), 1UL);
    mutator = mutators[0].get();
    ASSERT_EQ(mutator->getUniqueIdentifier(), "math_mul_mutator");
  }

  {
    mutators = factory.mutators({"math_sub_mutator"});
    ASSERT_EQ(mutators.size(), 1UL);
    mutator = mutators[0].get();
    ASSERT_EQ(mutator->getUniqueIdentifier(), "math_sub_mutator");
  }

  {
    mutators = factory.mutators({"and_or_replacement_mutator"});
    ASSERT_EQ(mutators.size(), 1UL);
    mutator = mutators[0].get();
    ASSERT_EQ(mutator->getUniqueIdentifier(), "and_or_replacement_mutator");
  }

  {
    mutators = factory.mutators({"negate_mutator"});
    ASSERT_EQ(mutators.size(), 1UL);
    mutator = mutators[0].get();
    ASSERT_EQ(mutator->getUniqueIdentifier(), "negate_mutator");
  }

  {
    mutators = factory.mutators({"remove_void_function_mutator"});
    ASSERT_EQ(mutators.size(), 1UL);
    mutator = mutators[0].get();
    ASSERT_EQ(mutator->getUniqueIdentifier(), "remove_void_function_mutator");
  }

  {
    mutators = factory.mutators({"replace_call_mutator"});
    ASSERT_EQ(mutators.size(), 1UL);
    mutator = mutators[0].get();
    ASSERT_EQ(mutator->getUniqueIdentifier(), "replace_call_mutator");
  }

  {
    mutators = factory.mutators({"scalar_value_mutator"});
    ASSERT_EQ(mutators.size(), 1UL);
    mutator = mutators[0].get();
    ASSERT_EQ(mutator->getUniqueIdentifier(), "scalar_value_mutator");
  }
}

static std::function<bool(unique_ptr<Mutator> &)> predicate(const char *name) {
  return [=](const unique_ptr<Mutator> &o) {
    return o->getUniqueIdentifier() == name;
  };
}

TEST(MutatorsFactory, CompositeMutators) {
  MutatorsFactory factory;
  vector<unique_ptr<Mutator>> mutators;
  vector<unique_ptr<Mutator>>::iterator searchResult;

  {
    mutators = factory.mutators({"math"});
    ASSERT_EQ(mutators.size(), 4UL);

    searchResult = find_if(mutators.begin(), mutators.end(),
                           predicate("math_add_mutator"));
    ASSERT_NE(searchResult, mutators.end());
    searchResult = find_if(mutators.begin(), mutators.end(),
                           predicate("math_div_mutator"));
    ASSERT_NE(searchResult, mutators.end());
    searchResult = find_if(mutators.begin(), mutators.end(),
                           predicate("math_mul_mutator"));
    ASSERT_NE(searchResult, mutators.end());
    searchResult = find_if(mutators.begin(), mutators.end(),
                           predicate("math_sub_mutator"));
    ASSERT_NE(searchResult, mutators.end());
  }

  {
    mutators = factory.mutators({"functions"});
    ASSERT_EQ(mutators.size(), 2UL);

    searchResult = find_if(mutators.begin(), mutators.end(),
                           predicate("remove_void_function_mutator"));
    ASSERT_NE(searchResult, mutators.end());
    searchResult = find_if(mutators.begin(), mutators.end(),
                           predicate("replace_call_mutator"));
    ASSERT_NE(searchResult, mutators.end());
  }

  {
    mutators = factory.mutators({"constant"});
    ASSERT_EQ(mutators.size(), 1UL);

    searchResult = find_if(mutators.begin(), mutators.end(),
                           predicate("scalar_value_mutator"));
    ASSERT_NE(searchResult, mutators.end());
  }

  {
    mutators = factory.mutators({"default"});
    ASSERT_EQ(mutators.size(), 3UL);

    searchResult = find_if(mutators.begin(), mutators.end(),
                           predicate("math_add_mutator"));
    ASSERT_NE(searchResult, mutators.end());
    searchResult =
        find_if(mutators.begin(), mutators.end(), predicate("negate_mutator"));
    ASSERT_NE(searchResult, mutators.end());
    searchResult = find_if(mutators.begin(), mutators.end(),
                           predicate("remove_void_function_mutator"));
    ASSERT_NE(searchResult, mutators.end());
  }

  {
    mutators = factory.mutators({"experimental"});
    ASSERT_EQ(mutators.size(), 17UL);

    searchResult = find_if(mutators.begin(), mutators.end(),
                           predicate("math_div_mutator"));
    ASSERT_NE(searchResult, mutators.end());
    searchResult = find_if(mutators.begin(), mutators.end(),
                           predicate("math_mul_mutator"));
    ASSERT_NE(searchResult, mutators.end());
    searchResult = find_if(mutators.begin(), mutators.end(),
                           predicate("math_sub_mutator"));
    ASSERT_NE(searchResult, mutators.end());
    searchResult = find_if(mutators.begin(), mutators.end(),
                           predicate("and_or_replacement_mutator"));
    ASSERT_NE(searchResult, mutators.end());
    searchResult = find_if(mutators.begin(), mutators.end(),
                           predicate("replace_assignment_mutator"));
    ASSERT_NE(searchResult, mutators.end());
    searchResult = find_if(mutators.begin(), mutators.end(),
                           predicate("replace_call_mutator"));
    ASSERT_NE(searchResult, mutators.end());
    searchResult = find_if(mutators.begin(), mutators.end(),
                           predicate("scalar_value_mutator"));
    ASSERT_NE(searchResult, mutators.end());
  }

  {
    mutators = factory.mutators({"all"});
    ASSERT_EQ(mutators.size(), 20UL);

    searchResult = find_if(mutators.begin(), mutators.end(),
                           predicate("math_add_mutator"));
    ASSERT_NE(searchResult, mutators.end());
    searchResult = find_if(mutators.begin(), mutators.end(),
                           predicate("math_div_mutator"));
    ASSERT_NE(searchResult, mutators.end());
    searchResult = find_if(mutators.begin(), mutators.end(),
                           predicate("math_mul_mutator"));
    ASSERT_NE(searchResult, mutators.end());
    searchResult = find_if(mutators.begin(), mutators.end(),
                           predicate("math_sub_mutator"));
    ASSERT_NE(searchResult, mutators.end());
    searchResult = find_if(mutators.begin(), mutators.end(),
                           predicate("and_or_replacement_mutator"));
    ASSERT_NE(searchResult, mutators.end());
    searchResult =
        find_if(mutators.begin(), mutators.end(), predicate("negate_mutator"));
    ASSERT_NE(searchResult, mutators.end());
    searchResult = find_if(mutators.begin(), mutators.end(),
                           predicate("remove_void_function_mutator"));
    ASSERT_NE(searchResult, mutators.end());
    searchResult = find_if(mutators.begin(), mutators.end(),
                           predicate("replace_assignment_mutator"));
    ASSERT_NE(searchResult, mutators.end());
    searchResult = find_if(mutators.begin(), mutators.end(),
                           predicate("replace_call_mutator"));
    ASSERT_NE(searchResult, mutators.end());
    searchResult = find_if(mutators.begin(), mutators.end(),
                           predicate("scalar_value_mutator"));
    ASSERT_NE(searchResult, mutators.end());
  }

  {
    mutators = factory.mutators({"default", "experimental"});
    ASSERT_EQ(mutators.size(), 20UL);

    searchResult = find_if(mutators.begin(), mutators.end(),
                           predicate("math_add_mutator"));
    ASSERT_NE(searchResult, mutators.end());
    searchResult = find_if(mutators.begin(), mutators.end(),
                           predicate("math_div_mutator"));
    ASSERT_NE(searchResult, mutators.end());
    searchResult = find_if(mutators.begin(), mutators.end(),
                           predicate("math_mul_mutator"));
    ASSERT_NE(searchResult, mutators.end());
    searchResult = find_if(mutators.begin(), mutators.end(),
                           predicate("math_sub_mutator"));
    ASSERT_NE(searchResult, mutators.end());
    searchResult = find_if(mutators.begin(), mutators.end(),
                           predicate("and_or_replacement_mutator"));
    ASSERT_NE(searchResult, mutators.end());
    searchResult =
        find_if(mutators.begin(), mutators.end(), predicate("negate_mutator"));
    ASSERT_NE(searchResult, mutators.end());
    searchResult = find_if(mutators.begin(), mutators.end(),
                           predicate("remove_void_function_mutator"));
    ASSERT_NE(searchResult, mutators.end());
    searchResult = find_if(mutators.begin(), mutators.end(),
                           predicate("replace_assignment_mutator"));
    ASSERT_NE(searchResult, mutators.end());
    searchResult = find_if(mutators.begin(), mutators.end(),
                           predicate("replace_call_mutator"));
    ASSERT_NE(searchResult, mutators.end());
    searchResult = find_if(mutators.begin(), mutators.end(),
                           predicate("scalar_value_mutator"));
    ASSERT_NE(searchResult, mutators.end());
  }
}

TEST(MutatorsFactory, UniqueMutators) {
  MutatorsFactory factory;
  vector<unique_ptr<Mutator>> mutators;
  vector<unique_ptr<Mutator>>::iterator searchResult;
  Mutator *mutator = nullptr;

  {
    mutators = factory.mutators({"math_add_mutator", "math_add_mutator"});
    ASSERT_EQ(mutators.size(), 1UL);
    mutator = mutators[0].get();
    ASSERT_EQ(mutator->getUniqueIdentifier(), "math_add_mutator");
  }

  {
    mutators = factory.mutators({"math", "math_add_mutator"});
    ASSERT_EQ(mutators.size(), 4UL);

    searchResult = find_if(mutators.begin(), mutators.end(),
                           predicate("math_add_mutator"));
    ASSERT_NE(searchResult, mutators.end());
    searchResult = find_if(mutators.begin(), mutators.end(),
                           predicate("math_div_mutator"));
    ASSERT_NE(searchResult, mutators.end());
    searchResult = find_if(mutators.begin(), mutators.end(),
                           predicate("math_mul_mutator"));
    ASSERT_NE(searchResult, mutators.end());
    searchResult = find_if(mutators.begin(), mutators.end(),
                           predicate("math_sub_mutator"));
    ASSERT_NE(searchResult, mutators.end());
  }
}
