#include "MutantExtractor.h"
#include <LLVMCompatibility.h>
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
    diagnostics.error("Cannot read executable:"s + maybeBuffer.getError().message());
    return mutants;
  }
  llvm::MemoryBuffer *buffer = maybeBuffer->get();
  llvm::Expected<std::unique_ptr<llvm::object::ObjectFile>> maybeObject =
      llvm::object::ObjectFile::createObjectFile(buffer->getMemBufferRef());
  if (!maybeObject) {
    llvm::Error error = maybeObject.takeError();
    diagnostics.error("Executable is not an object file: "s + llvm::toString(std::move(error)));
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
        std::string identifier = encoding.substr(0, encoding.rfind(':'));
        std::string mutator = chunks[0];
        std::string location = chunks[1];
        int line = std::stoi(chunks[2]);
        int column = std::stoi(chunks[3]);
        bool covered = std::stoi(chunks[4]);
        auto mutant =
            std::make_unique<Mutant>(identifier,
                                     mutator,
                                     mull::SourceLocation("", location, "", location, line, column),
                                     covered);
        mutants.push_back(std::move(mutant));
      }
    }
  }

  return mutants;
}
