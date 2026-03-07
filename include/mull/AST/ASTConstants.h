#pragma once

#include "mull/Mutators/MutatorKind.h"

#include <clang/AST/Expr.h>

#include <vector>

namespace mull {
extern std::vector<std::tuple<clang::BinaryOperator::Opcode, mull::MutatorKind,
                              clang::BinaryOperator::Opcode, std::string>>
    BinaryMutations;
extern std::vector<std::tuple<clang::UnaryOperator::Opcode, mull::MutatorKind, std::string>>
    UnaryMutations;
} // namespace mull
