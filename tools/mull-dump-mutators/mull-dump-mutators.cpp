/// Tool to dump available mutators in RST format for documentation generation
#include <mull/Mutators/Mutator.h>
#include <mull/Mutators/MutatorsFactory.h>

#include "rust/mull-cxx-bridge/bridge.rs.h"

#include <llvm/Support/FileSystem.h>
#include <llvm/Support/raw_ostream.h>

#include <algorithm>
#include <cstdio>
#include <sstream>
#include <string>
#include <vector>

using namespace mull;

static void dumpMutators(const MullDiagnostics &diagnostics, const std::string &out) {
  MutatorsFactory factory(diagnostics);
  factory.init();

  std::vector<Mutator *> availableMutators;
  for (const auto &pair : factory.getMutatorsMapping()) {
    availableMutators.push_back(pair.second.get());
  }

  std::sort(availableMutators.begin(), availableMutators.end(), [&](Mutator *lhs, Mutator *rhs) {
    return lhs->getUniqueIdentifier() < rhs->getUniqueIdentifier();
  });

  auto sanitizeString = [](const std::string &input) -> std::string {
    std::string result = input;
    auto replaceOne = [&](const std::string &replacement) {
      std::string escape("\\");
      auto pos = result.find(replacement);
      if (pos != std::string::npos) {
        result.replace(pos, replacement.size(), escape + replacement);
      }
    };
    replaceOne("|=");
    replaceOne("*=");
    return result;
  };

  std::stringstream replacements;
  std::stringstream table;

  std::string operatorHeaderName("Operator Name");
  std::string operatorHeader(operatorHeaderName.size(), '=');
  std::string semanticsHeaderName("Operator Semantics");
  std::string semanticsHeader(semanticsHeaderName.size(), '=');

  table << operatorHeader << " " << semanticsHeader << "\n";
  table << operatorHeaderName << " " << semanticsHeaderName << "\n";
  table << operatorHeader << " " << semanticsHeader << "\n";

  for (size_t i = 0; i < availableMutators.size(); i++) {
    Mutator *mutator = availableMutators[i];
    std::string n = std::to_string(i);
    std::string name("op" + n);
    std::string description("desc" + n);

    std::string padding(operatorHeader.size() - name.size() - 1, ' ');
    table << "|" << name << "|" << padding << "|" << description << "|\n";

    replacements << ".. |" << name << "| replace:: " << mutator->getUniqueIdentifier() << "\n";
    replacements << ".. |" << description
                 << "| replace:: " << sanitizeString(mutator->getDescription()) << "\n";
  }

  replacements << "\n\n";
  table << operatorHeader << " " << semanticsHeader << "\n";

  table << "\n\n";
  table << "Groups:\n";
  rust::Vec<MutatorGroupDef> groupDefs = get_mutator_group_definitions();
  for (const auto &def : groupDefs) {
    table << "    :" << std::string(def.name) << ":\t" << std::string(def.description) << "\n\n";
  }

  std::error_code ec;
  llvm::raw_fd_ostream os(out, ec);
  if (ec) {
    diagnostics.error(ec.message());
  }
  os << replacements.str();
  auto s = table.str();
  // trim trailing newline
  os << s.substr(0, s.size() - 1);
}

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "Usage: %s <output-file>\n", argv[0]);
    return 1;
  }

  auto core = init_core_ffi();
  dumpMutators(core->diag(), argv[1]);
  return 0;
}
