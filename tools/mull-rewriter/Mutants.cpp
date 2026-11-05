#include "Mutants.h"

#include <llvm/ADT/StringSwitch.h>
#include <sqlite3.h>

namespace mull_rewriter {

std::optional<MutatorTarget> targetOf(llvm::StringRef mutator) {
  return llvm::StringSwitch<std::optional<MutatorTarget>>(mutator)
      .Case("cxx_add_to_sub", MutatorTarget{ NodeKind::Binary, "+" })
      .Case("cxx_sub_to_add", MutatorTarget{ NodeKind::Binary, "-" })
      .Case("cxx_mul_to_div", MutatorTarget{ NodeKind::Binary, "*" })
      .Case("cxx_div_to_mul", MutatorTarget{ NodeKind::Binary, "/" })
      .Case("cxx_rem_to_div", MutatorTarget{ NodeKind::Binary, "%" })
      .Case("cxx_lshift_to_rshift", MutatorTarget{ NodeKind::Binary, "<<" })
      .Case("cxx_rshift_to_lshift", MutatorTarget{ NodeKind::Binary, ">>" })
      .Case("cxx_and_to_or", MutatorTarget{ NodeKind::Binary, "&" })
      .Case("cxx_or_to_and", MutatorTarget{ NodeKind::Binary, "|" })
      .Case("cxx_xor_to_or", MutatorTarget{ NodeKind::Binary, "^" })
      .Case("cxx_eq_to_ne", MutatorTarget{ NodeKind::Binary, "==" })
      .Case("cxx_ne_to_eq", MutatorTarget{ NodeKind::Binary, "!=" })
      .Case("cxx_lt_to_le", MutatorTarget{ NodeKind::Binary, "<" })
      .Case("cxx_lt_to_ge", MutatorTarget{ NodeKind::Binary, "<" })
      .Case("cxx_le_to_lt", MutatorTarget{ NodeKind::Binary, "<=" })
      .Case("cxx_le_to_gt", MutatorTarget{ NodeKind::Binary, "<=" })
      .Case("cxx_gt_to_ge", MutatorTarget{ NodeKind::Binary, ">" })
      .Case("cxx_gt_to_le", MutatorTarget{ NodeKind::Binary, ">" })
      .Case("cxx_ge_to_gt", MutatorTarget{ NodeKind::Binary, ">=" })
      .Case("cxx_ge_to_lt", MutatorTarget{ NodeKind::Binary, ">=" })
      .Case("cxx_add_assign_to_sub_assign", MutatorTarget{ NodeKind::Binary, "+=" })
      .Case("cxx_sub_assign_to_add_assign", MutatorTarget{ NodeKind::Binary, "-=" })
      .Case("cxx_mul_assign_to_div_assign", MutatorTarget{ NodeKind::Binary, "*=" })
      .Case("cxx_div_assign_to_mul_assign", MutatorTarget{ NodeKind::Binary, "/=" })
      .Case("cxx_rem_assign_to_div_assign", MutatorTarget{ NodeKind::Binary, "%=" })
      .Case("cxx_lshift_assign_to_rshift_assign", MutatorTarget{ NodeKind::Binary, "<<=" })
      .Case("cxx_rshift_assign_to_lshift_assign", MutatorTarget{ NodeKind::Binary, ">>=" })
      .Case("cxx_and_assign_to_or_assign", MutatorTarget{ NodeKind::Binary, "&=" })
      .Case("cxx_or_assign_to_and_assign", MutatorTarget{ NodeKind::Binary, "|=" })
      .Case("cxx_xor_assign_to_or_assign", MutatorTarget{ NodeKind::Binary, "^=" })
      .Case("cxx_minus_to_noop", MutatorTarget{ NodeKind::UnaryPrefix, "-" })
      .Case("cxx_bitwise_not_to_noop", MutatorTarget{ NodeKind::UnaryPrefix, "~" })
      .Case("cxx_remove_negation", MutatorTarget{ NodeKind::UnaryPrefix, "!" })
      .Case("negate_mutator", MutatorTarget{ NodeKind::UnaryPrefix, "!" })
      .Case("cxx_pre_inc_to_pre_dec", MutatorTarget{ NodeKind::UnaryPrefix, "++" })
      .Case("cxx_pre_dec_to_pre_inc", MutatorTarget{ NodeKind::UnaryPrefix, "--" })
      .Case("cxx_post_inc_to_post_dec", MutatorTarget{ NodeKind::UnaryPostfix, "++" })
      .Case("cxx_post_dec_to_post_inc", MutatorTarget{ NodeKind::UnaryPostfix, "--" })
      .Case("cxx_remove_void_call", MutatorTarget{ NodeKind::Call, "" })
      .Case("cxx_replace_scalar_call_zr", MutatorTarget{ NodeKind::Call, "" })
      .Case("cxx_replace_scalar_call_nz", MutatorTarget{ NodeKind::Call, "" })
      .Case("cxx_init_const", MutatorTarget{ NodeKind::Initializer, "" })
      .Case("cxx_assign_const", MutatorTarget{ NodeKind::Assignment, "=" })
      .Default(std::nullopt);
}

static std::string columnText(sqlite3_stmt *statement, int column) {
  const unsigned char *text = sqlite3_column_text(statement, column);
  return text ? reinterpret_cast<const char *>(text) : "";
}

std::optional<std::string> readMutants(const std::string &reportPath,
                                       std::vector<Mutant> &mutants) {
  sqlite3 *database = nullptr;
  if (sqlite3_open_v2(reportPath.c_str(), &database, SQLITE_OPEN_READONLY, nullptr) != SQLITE_OK) {
    std::string error = sqlite3_errmsg(database);
    sqlite3_close(database);
    return "cannot open '" + reportPath + "': " + error;
  }

  const char *query = "SELECT mutant_id, mutator, filename, line_number, column_number, "
                      "end_line_number, end_column_number, mutation_replacement FROM mutant";
  sqlite3_stmt *statement = nullptr;
  if (sqlite3_prepare_v2(database, query, -1, &statement, nullptr) != SQLITE_OK) {
    std::string error = sqlite3_errmsg(database);
    sqlite3_close(database);
    return "cannot read mutants from '" + reportPath + "': " + error;
  }

  while (sqlite3_step(statement) == SQLITE_ROW) {
    mutants.push_back(Mutant{ columnText(statement, 0),
                              columnText(statement, 1),
                              columnText(statement, 2),
                              static_cast<unsigned>(sqlite3_column_int(statement, 3)),
                              static_cast<unsigned>(sqlite3_column_int(statement, 4)),
                              static_cast<unsigned>(sqlite3_column_int(statement, 5)),
                              static_cast<unsigned>(sqlite3_column_int(statement, 6)),
                              columnText(statement, 7) });
  }
  sqlite3_finalize(statement);
  sqlite3_close(database);
  return std::nullopt;
}

} // namespace mull_rewriter
