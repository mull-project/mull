#include "MutantExtractor.h"
#include "ObjectFile.h"
#include <llvm/Object/ObjectFile.h>
#include <sstream>
#include <unordered_set>

using namespace mull;
using namespace std::string_literals;

static llvm::StringRef getSectionName(const llvm::object::SectionRef &section) {
  llvm::Expected<llvm::StringRef> name = section.getName();
  if (!name) {
    return {};
  }
  return name.get();
}

static std::vector<std::string> split(const std::string &input, char delimiter) {
  std::vector<std::string> output;
  std::string string;
  std::istringstream stream(input);
  while (std::getline(stream, string, delimiter)) {
    if (!string.empty()) {
      output.push_back(string);
    }
  }
  return output;
}

MutantExtractor::MutantExtractor(Diagnostics &diagnostics) : diagnostics(diagnostics) {}

std::vector<std::string> MutantExtractor::extractMutants(const std::string &executable) {
  auto [buffer, objectFile] = loadObjectFile(diagnostics, executable);
  if (!objectFile) {
    diagnostics.warning("Skipping: "s + executable);
    return {};
  }
  for (auto &section : objectFile->sections()) {
    llvm::StringRef name = getSectionName(section);
    if (name.equals(".mull_mutants")) {
      llvm::Expected<llvm::StringRef> content = section.getContents();
      if (!content) {
        return {};
      }
      return split(content.get().str(), '\0');
    }
  }

  return {};
}

std::vector<std::unique_ptr<Mutant>>
MutantExtractor::extractMutants(const std::vector<std::string> &mutantHolders) {
  std::vector<std::string> encodings;
  for (auto &holder : mutantHolders) {
    auto mutants = extractMutants(holder);
    std::move(std::begin(mutants), std::end(mutants), std::back_inserter(encodings));
  }

  std::unordered_set<std::string> deduplicatedEncodings;
  for (auto &encoding : encodings) {
    deduplicatedEncodings.insert(encoding);
  }
  std::vector<std::unique_ptr<Mutant>> mutants;
  for (auto &encoding : deduplicatedEncodings) {
    std::vector<std::string> chunks = split(encoding, ':');

    std::string mutator = chunks[0];
    std::string location = chunks[1];
    int beginLine = std::stoi(chunks[2]);
    int beginColumn = std::stoi(chunks[3]);
    int endLine = std::stoi(chunks[4]);
    int endColumn = std::stoi(chunks[5]);

    std::ostringstream mis;
    mis << mutator << ":" << location << ":" << beginLine << ":" << beginColumn;
    std::string identifier = mis.str();

    auto mutant = std::make_unique<Mutant>(
        identifier,
        mutator,
        mull::SourceLocation("", location, "", location, beginLine, beginColumn),
        mull::SourceLocation("", location, "", location, endLine, endColumn));
    mutants.push_back(std::move(mutant));
  }

  std::sort(std::begin(mutants), std::end(mutants), MutantComparator());

  return mutants;
}
