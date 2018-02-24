#include "MutationOperators/MutationOperatorsFactory.h"

#include "gtest/gtest.h"

#include <vector>

using namespace mull;
using namespace llvm;
using namespace std;

TEST(MutationsOperatorFactory, SingleOperators) {
  MutationOperatorsFactory factory;
  vector<unique_ptr<MutationOperator>> operators;
  MutationOperator *mutationOperator = nullptr;

  {
    operators = factory.mutationOperators({ "math_add_mutation_operator" });
    ASSERT_EQ(operators.size(), 1UL);
    mutationOperator = operators[0].get();
    ASSERT_EQ(mutationOperator->uniqueID(), "math_add_mutation_operator");
  }

  {
    operators = factory.mutationOperators({ "math_div_mutation_operator" });
    ASSERT_EQ(operators.size(), 1UL);
    mutationOperator = operators[0].get();
    ASSERT_EQ(mutationOperator->uniqueID(), "math_div_mutation_operator");
  }

  {
    operators = factory.mutationOperators({ "math_mul_mutation_operator" });
    ASSERT_EQ(operators.size(), 1UL);
    mutationOperator = operators[0].get();
    ASSERT_EQ(mutationOperator->uniqueID(), "math_mul_mutation_operator");
  }

  {
    operators = factory.mutationOperators({ "math_sub_mutation_operator" });
    ASSERT_EQ(operators.size(), 1UL);
    mutationOperator = operators[0].get();
    ASSERT_EQ(mutationOperator->uniqueID(), "math_sub_mutation_operator");
  }

  {
    operators = factory.mutationOperators({ "and_or_replacement_mutation_operator" });
    ASSERT_EQ(operators.size(), 1UL);
    mutationOperator = operators[0].get();
    ASSERT_EQ(mutationOperator->uniqueID(), "and_or_replacement_mutation_operator");
  }

  {
    operators = factory.mutationOperators({ "negate_mutation_operator" });
    ASSERT_EQ(operators.size(), 1UL);
    mutationOperator = operators[0].get();
    ASSERT_EQ(mutationOperator->uniqueID(), "negate_mutation_operator");
  }

  {
    operators = factory.mutationOperators({ "remove_void_function_mutation_operator" });
    ASSERT_EQ(operators.size(), 1UL);
    mutationOperator = operators[0].get();
    ASSERT_EQ(mutationOperator->uniqueID(), "remove_void_function_mutation_operator");
  }

  {
    operators = factory.mutationOperators({ "replace_call_mutation_operator" });
    ASSERT_EQ(operators.size(), 1UL);
    mutationOperator = operators[0].get();
    ASSERT_EQ(mutationOperator->uniqueID(), "replace_call_mutation_operator");
  }

  {
    operators = factory.mutationOperators({ "scalar_value_mutation_operator" });
    ASSERT_EQ(operators.size(), 1UL);
    mutationOperator = operators[0].get();
    ASSERT_EQ(mutationOperator->uniqueID(), "scalar_value_mutation_operator");
  }
}

static std::function<bool (unique_ptr<MutationOperator>&)>
predicate(const char *name) {
  return [=](const unique_ptr<MutationOperator>& o) {
    return o->uniqueID() == name;
  };
}

TEST(MutationsOperatorFactory, CompositeOperators) {
  MutationOperatorsFactory factory;
  vector<unique_ptr<MutationOperator>> operators;
  vector<unique_ptr<MutationOperator>>::iterator searchResult;

  {
    operators = factory.mutationOperators({ "math" });
    ASSERT_EQ(operators.size(), 4UL);

    searchResult = find_if(operators.begin(), operators.end(), predicate("math_add_mutation_operator"));
    ASSERT_NE(searchResult, operators.end());
    searchResult = find_if(operators.begin(), operators.end(), predicate("math_div_mutation_operator"));
    ASSERT_NE(searchResult, operators.end());
    searchResult = find_if(operators.begin(), operators.end(), predicate("math_mul_mutation_operator"));
    ASSERT_NE(searchResult, operators.end());
    searchResult = find_if(operators.begin(), operators.end(), predicate("math_sub_mutation_operator"));
    ASSERT_NE(searchResult, operators.end());
  }

  {
    operators = factory.mutationOperators({ "conditional" });
    ASSERT_EQ(operators.size(), 3UL);

    searchResult = find_if(operators.begin(), operators.end(), predicate("and_or_replacement_mutation_operator"));
    ASSERT_NE(searchResult, operators.end());
    searchResult = find_if(operators.begin(), operators.end(), predicate("negate_mutation_operator"));
    ASSERT_NE(searchResult, operators.end());
    searchResult = find_if(operators.begin(), operators.end(), predicate("conditionals_boundary_mutator"));
    ASSERT_NE(searchResult, operators.end());
  }

  {
    operators = factory.mutationOperators({ "functions" });
    ASSERT_EQ(operators.size(), 2UL);

    searchResult = find_if(operators.begin(), operators.end(), predicate("remove_void_function_mutation_operator"));
    ASSERT_NE(searchResult, operators.end());
    searchResult = find_if(operators.begin(), operators.end(), predicate("replace_call_mutation_operator"));
    ASSERT_NE(searchResult, operators.end());
  }

  {
    operators = factory.mutationOperators({ "constant" });
    ASSERT_EQ(operators.size(), 1UL);

    searchResult = find_if(operators.begin(), operators.end(), predicate("scalar_value_mutation_operator"));
    ASSERT_NE(searchResult, operators.end());
  }

  {
    operators = factory.mutationOperators({ "default" });
    ASSERT_EQ(operators.size(), 3UL);

    searchResult = find_if(operators.begin(), operators.end(), predicate("math_add_mutation_operator"));
    ASSERT_NE(searchResult, operators.end());
    searchResult = find_if(operators.begin(), operators.end(), predicate("negate_mutation_operator"));
    ASSERT_NE(searchResult, operators.end());
    searchResult = find_if(operators.begin(), operators.end(), predicate("remove_void_function_mutation_operator"));
    ASSERT_NE(searchResult, operators.end());
  }

  {
    operators = factory.mutationOperators({ "experimental" });
    ASSERT_EQ(operators.size(), 8UL);

    searchResult = find_if(operators.begin(), operators.end(), predicate("math_div_mutation_operator"));
    ASSERT_NE(searchResult, operators.end());
    searchResult = find_if(operators.begin(), operators.end(), predicate("math_mul_mutation_operator"));
    ASSERT_NE(searchResult, operators.end());
    searchResult = find_if(operators.begin(), operators.end(), predicate("math_sub_mutation_operator"));
    ASSERT_NE(searchResult, operators.end());
    searchResult = find_if(operators.begin(), operators.end(), predicate("and_or_replacement_mutation_operator"));
    ASSERT_NE(searchResult, operators.end());
    searchResult = find_if(operators.begin(), operators.end(), predicate("replace_assignment_mutation_operator"));
    ASSERT_NE(searchResult, operators.end());
    searchResult = find_if(operators.begin(), operators.end(), predicate("replace_call_mutation_operator"));
    ASSERT_NE(searchResult, operators.end());
    searchResult = find_if(operators.begin(), operators.end(), predicate("scalar_value_mutation_operator"));
    ASSERT_NE(searchResult, operators.end());
    searchResult = find_if(operators.begin(), operators.end(), predicate("conditionals_boundary_mutator"));
    ASSERT_NE(searchResult, operators.end());
  }

  {
    operators = factory.mutationOperators({ "all" });
    ASSERT_EQ(operators.size(), 11UL);

    searchResult = find_if(operators.begin(), operators.end(), predicate("math_add_mutation_operator"));
    ASSERT_NE(searchResult, operators.end());
    searchResult = find_if(operators.begin(), operators.end(), predicate("math_div_mutation_operator"));
    ASSERT_NE(searchResult, operators.end());
    searchResult = find_if(operators.begin(), operators.end(), predicate("math_mul_mutation_operator"));
    ASSERT_NE(searchResult, operators.end());
    searchResult = find_if(operators.begin(), operators.end(), predicate("math_sub_mutation_operator"));
    ASSERT_NE(searchResult, operators.end());
    searchResult = find_if(operators.begin(), operators.end(), predicate("and_or_replacement_mutation_operator"));
    ASSERT_NE(searchResult, operators.end());
    searchResult = find_if(operators.begin(), operators.end(), predicate("negate_mutation_operator"));
    ASSERT_NE(searchResult, operators.end());
    searchResult = find_if(operators.begin(), operators.end(), predicate("remove_void_function_mutation_operator"));
    ASSERT_NE(searchResult, operators.end());
    searchResult = find_if(operators.begin(), operators.end(), predicate("replace_assignment_mutation_operator"));
    ASSERT_NE(searchResult, operators.end());
    searchResult = find_if(operators.begin(), operators.end(), predicate("replace_call_mutation_operator"));
    ASSERT_NE(searchResult, operators.end());
    searchResult = find_if(operators.begin(), operators.end(), predicate("scalar_value_mutation_operator"));
    ASSERT_NE(searchResult, operators.end());
    searchResult = find_if(operators.begin(), operators.end(), predicate("conditionals_boundary_mutator"));
    ASSERT_NE(searchResult, operators.end());
  }

  {
    operators = factory.mutationOperators({ "default", "experimental" });
    ASSERT_EQ(operators.size(), 11UL);

    searchResult = find_if(operators.begin(), operators.end(), predicate("math_add_mutation_operator"));
    ASSERT_NE(searchResult, operators.end());
    searchResult = find_if(operators.begin(), operators.end(), predicate("math_div_mutation_operator"));
    ASSERT_NE(searchResult, operators.end());
    searchResult = find_if(operators.begin(), operators.end(), predicate("math_mul_mutation_operator"));
    ASSERT_NE(searchResult, operators.end());
    searchResult = find_if(operators.begin(), operators.end(), predicate("math_sub_mutation_operator"));
    ASSERT_NE(searchResult, operators.end());
    searchResult = find_if(operators.begin(), operators.end(), predicate("and_or_replacement_mutation_operator"));
    ASSERT_NE(searchResult, operators.end());
    searchResult = find_if(operators.begin(), operators.end(), predicate("negate_mutation_operator"));
    ASSERT_NE(searchResult, operators.end());
    searchResult = find_if(operators.begin(), operators.end(), predicate("remove_void_function_mutation_operator"));
    ASSERT_NE(searchResult, operators.end());
    searchResult = find_if(operators.begin(), operators.end(), predicate("replace_assignment_mutation_operator"));
    ASSERT_NE(searchResult, operators.end());
    searchResult = find_if(operators.begin(), operators.end(), predicate("replace_call_mutation_operator"));
    ASSERT_NE(searchResult, operators.end());
    searchResult = find_if(operators.begin(), operators.end(), predicate("scalar_value_mutation_operator"));
    ASSERT_NE(searchResult, operators.end());
  }
}

TEST(MutationsOperatorFactory, UniqueOperators) {
  MutationOperatorsFactory factory;
  vector<unique_ptr<MutationOperator>> operators;
  vector<unique_ptr<MutationOperator>>::iterator searchResult;
  MutationOperator *mutationOperator = nullptr;

  {
    operators = factory.mutationOperators({
      "math_add_mutation_operator",
      "math_add_mutation_operator"
    });
    ASSERT_EQ(operators.size(), 1UL);
    mutationOperator = operators[0].get();
    ASSERT_EQ(mutationOperator->uniqueID(), "math_add_mutation_operator");
  }

  {
    operators = factory.mutationOperators({
      "math",
      "math_add_mutation_operator"
    });
    ASSERT_EQ(operators.size(), 4UL);

    searchResult = find_if(operators.begin(), operators.end(), predicate("math_add_mutation_operator"));
    ASSERT_NE(searchResult, operators.end());
    searchResult = find_if(operators.begin(), operators.end(), predicate("math_div_mutation_operator"));
    ASSERT_NE(searchResult, operators.end());
    searchResult = find_if(operators.begin(), operators.end(), predicate("math_mul_mutation_operator"));
    ASSERT_NE(searchResult, operators.end());
    searchResult = find_if(operators.begin(), operators.end(), predicate("math_sub_mutation_operator"));
    ASSERT_NE(searchResult, operators.end());
  }
}
