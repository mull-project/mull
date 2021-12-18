#include "MutantExtractor.h"
#include <LLVMCompatibility.h>
#include <iostream>
#include <llvm/Object/ObjectFile.h>
#include <sstream>

using namespace mull;
using namespace std::string_literals;

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

std::vector<std::unique_ptr<Mutant>>
MutantExtractor::extractMutants(const std::string &executable) {
  std::vector<std::unique_ptr<Mutant>> mutants;

  llvm::ErrorOr<std::unique_ptr<llvm::MemoryBuffer>> maybeBuffer =
      llvm::MemoryBuffer::getFile(executable);
  if (!maybeBuffer) {
    diagnostics.error("Cannot read executable: "s + maybeBuffer.getError().message() + ": " +
                      executable);
    return mutants;
  }
  llvm::MemoryBuffer *buffer = maybeBuffer->get();
  llvm::Expected<std::unique_ptr<llvm::object::ObjectFile>> maybeObject =
      llvm::object::ObjectFile::createObjectFile(buffer->getMemBufferRef());
  if (!maybeObject) {
    llvm::Error error = maybeObject.takeError();
    /// On older versions of macOS we fail to load certain system libraries because they are fat
    /// libraries On newer versions of macOS we never reach this line because the system libraries
    /// live in cache and cannot be read from FS This should be an error, but we relax it to a
    /// warning to not fail on macOS
    /// TODO: we should probably add support for universal binaries at some point
    /// https://github.com/mull-project/mull/issues/932
    diagnostics.warning("Skipping. Executable is not an object file: "s +
                        llvm::toString(std::move(error)) + ": " + executable);
    return mutants;
  }

  llvm::object::ObjectFile *objectFile = maybeObject->get();
  for (auto &section : objectFile->sections()) {
    llvm::StringRef name = llvm_compat::getSectionName(section);
    if (name.equals(".mull_mutants")) {
      llvm::StringRef content = llvm_compat::getSectionContent(section);
      std::vector<std::string> mutantEncodings = split(content.str(), '\0');
      for (auto &encoding : mutantEncodings) {
        std::vector<std::string> chunks = split(encoding, ':');

        std::string mutator = chunks[0];
        std::string location = chunks[1];
        int beginLine = std::stoi(chunks[2]);
        int beginColumn = std::stoi(chunks[3]);
        int endLine = std::stoi(chunks[4]);
        int endColumn = std::stoi(chunks[5]);
        bool covered = std::stoi(chunks[6]);

        std::ostringstream mis;
        mis << mutator << ":" << location << ":" << beginLine << ":" << beginColumn;
        std::string identifier = mis.str();

        auto mutant = std::make_unique<Mutant>(
            identifier,
            mutator,
            mull::SourceLocation("", location, "", location, beginLine, beginColumn),
            mull::SourceLocation("", location, "", location, endLine, endColumn),
            covered);
        mutants.push_back(std::move(mutant));
      }
    }
  }

  return mutants;
}

std::vector<std::unique_ptr<Mutant>>
MutantExtractor::extractMutants(const std::vector<std::string> &mutantHolders) {
  std::vector<std::unique_ptr<Mutant>> result;
  for (auto &holder : mutantHolders) {
    auto mutants = extractMutants(holder);
    std::move(std::begin(mutants), std::end(mutants), std::back_inserter(result));
  }
  return result;
}
