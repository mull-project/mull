Introduction to Mutation Testing
================================

Mutation Testing is a *fault-based* software testing technique. It evaluates the quality of a test suite by
calculating *mutation score* and showing gaps in *semantic coverage*. It does so by creating several
slightly modified versions of the original program, *mutants*, and running the test suite against each of them.
A mutant is considered to be *killed* if the test suite detects the change, or *survived* otherwise.
A mutant is killed if at least one of the tests starts failing.

Each mutation of original program is based on a set of *mutation operators* (or *mutators*). A mutator
is a predefined rule that either changes or removes an existing statement or expression in the original program.
Each rule is deterministic: the same set of mutation operators applied to the same program results in the
same set of mutants.

Mutation score is a ratio of killed vs total mutants. E.g., if seven out of ten mutants are killed,
then the score is 0.7, or 70%. The higher the score the better.
