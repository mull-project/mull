#include "JunkDetection/CXX/CXXJunkDetector.h"

#include "MutationPoint.h"
#include "Mutators/Mutator.h"
#include "Logger.h"

#include <llvm/IR/DebugInfoMetadata.h>
#include <llvm/IR/DebugLoc.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Instruction.h>
#include <llvm/Support/Path.h>
#include <clang/Tooling/CompilationDatabase.h>
#include <unistd.h>
#include <sys/param.h>

using namespace mull;
using namespace llvm;

#pragma mark - Debug

raw_ostream& operator<<(raw_ostream& stream, const CXString& str) {
  stream << clang_getCString(str);
  clang_disposeString(str);
  return stream;
}

void dump_cursor(CXCursor cursor, CXSourceLocation location, PhysicalAddress &address, MutationPoint *point) {
  errs() << point->getUniqueIdentifier() << "\n";
  point->getOriginalValue()->dump();
  Instruction *in = dyn_cast<Instruction>(point->getOriginalValue());
  errs() << in->getParent()->getParent()->getParent()->getModuleIdentifier() << "\n";

  errs() << address.filepath << ":" << address.line << ":" << address.column << "\n";
  CXCursorKind kind = clang_getCursorKind(cursor);
  errs() << "Kind '" << clang_getCursorKindSpelling(kind) << "'\n";

  if (kind == CXCursor_Namespace) {
    errs() << "not printing namespace contents\n";
    return;
  }

  CXSourceRange range = clang_getCursorExtent(cursor);
  CXSourceLocation begin = clang_getRangeStart(range);
  CXSourceLocation end = clang_getRangeEnd(range);

  unsigned int beginOffset = 0;
  unsigned int endOffset = 0;
  unsigned int origOffset = 0;
  clang_getFileLocation(begin, nullptr, nullptr, nullptr, &beginOffset);
  clang_getFileLocation(end, nullptr, nullptr, nullptr, &endOffset);
  clang_getFileLocation(location, nullptr, nullptr, nullptr, &origOffset);

  unsigned int offset = origOffset - beginOffset;

  auto length = endOffset - beginOffset;

  FILE *f = fopen(address.filepath.c_str(), "rb");

  fseek(f, beginOffset, SEEK_SET);
  char *buffer = new char[length + 1];
  fread(buffer, sizeof(char), length, f);

  buffer[length] = '\0';
  errs() << buffer << "\n";

  for (unsigned int i = 0; i < offset; i++) {
    errs() << " ";
  }
  errs() << "^\n";

  delete[] buffer;
}

#pragma mark - Phisycal Address

PhysicalAddress getAddress(MutationPoint *point) {
  PhysicalAddress address;

  if (auto instruction = dyn_cast<Instruction>(point->getOriginalValue())) {
    if (instruction->getMetadata(0)) {
      auto debugInfo = instruction->getDebugLoc();

      auto file = debugInfo->getFilename().str();
      auto directory = debugInfo->getDirectory().str();
      if (sys::path::is_absolute(file)) {
        address.filepath = file;
      } else {
        address.filepath = directory + sys::path::get_separator().str() + file;
      }

      address.directory = directory;
      address.line = debugInfo->getLine();
      address.column = debugInfo->getColumn();
    }
  }

  return address;
}

PhysicalAddress::PhysicalAddress() : directory(""), filepath(""), line(0), column(0) {}

bool PhysicalAddress::valid() {
  if (directory != "" && filepath != "" && line != 0 && column != 0) {
    return true;
  }

  return false;
}

#pragma mark - Junk Detector

class CHDir {
public:
  CHDir(const std::string &workDir) {
    char cwd[MAXPATHLEN];
    if (getcwd(cwd, sizeof(cwd)) == nullptr) {
      perror("getcwd");
    } else {
      prevWorkDir = std::string(cwd);
    }
    if (chdir(workDir.c_str()) == -1) {
      perror("chdir");
    }
  }
  ~CHDir() {
    if (chdir(prevWorkDir.c_str()) == -1) {
      perror("~chdir");
    }
  }
private:
  std::string prevWorkDir;
};

CXXJunkDetector::CXXJunkDetector() : index(clang_createIndex(true, true)) {
  using namespace clang::tooling;
  std::string buildDir("/opt/mull/llvm/build/");
  std::string error;
  compdb = CompilationDatabase::loadFromDirectory(buildDir, error);
  if (compdb == nullptr) {
    errs() << error << "\n";
  }
}

CXXJunkDetector::~CXXJunkDetector() {
  for (auto &pair : units) {
    clang_disposeTranslationUnit(pair.second);
  }
  clang_disposeIndex(index);
}

CXTranslationUnit
CXXJunkDetector::translationUnit(const std::string &directory,
                                 const std::string &sourceFile) {
  if (units.count(sourceFile) != 0) {
    return units[sourceFile];
  }

  int argc = 1;
  char **argv = nullptr;

  CHDir chdir(directory);

  auto commands = compdb->getCompileCommands(sourceFile);
  if (commands.empty()) {
    argv = new char*[argc];
    argv[0] = new char[1];
    argv[0][0] = '\0';
  } else {
    auto command = *commands.begin();
    argc = command.CommandLine.size();
    argv = new char*[argc];
    for (int i = 0; i < argc; i++) {
      auto commandLine = command.CommandLine[i];
      if (commandLine == "-c") {
        /// skipping this argument, otherwise clang can not parse AST
        /// for whatever reason...
        argv[i] = new char[1];
        argv[i][0] = '\0';
        argv[i + 1] = new char[1];
        argv[i + 1][0] = '\0';
        i++;
        continue;
      }
      argv[i] = new char[commandLine.size() + 1];
      strncpy(argv[i], commandLine.c_str(), commandLine.size());
      argv[i][commandLine.size()] = '\0';
    }
  }

  //    const char *argv[] = { nullptr };
  //    const int argc = sizeof(argv) / sizeof(argv[0]) - 1;
  CXTranslationUnit unit = nullptr;
  CXErrorCode code = clang_parseTranslationUnit2(index,
                                                 sourceFile.c_str(),
                                                 argv, argc,
                                                 nullptr, 0,
                                                 CXTranslationUnit_KeepGoing,
                                                 &unit);
  for (int i = 0; i < argc; i++) {
    delete [] argv[i];
  }
  delete [] argv;

  if (unit == nullptr) {
    Logger::error() << "Cannot parse translation unit: " << sourceFile << "\n";
    Logger::error() << "CXErrorCode: " << code << "\n";
    return nullptr;
  }
  units[sourceFile] = unit;
  return unit;
}

std::pair<CXCursor, CXSourceLocation>
CXXJunkDetector::cursorAndLocation(PhysicalAddress &address,
                                   MutationPoint *point) {
  Instruction *instruction = dyn_cast<Instruction>(point->getOriginalValue());
  if (instruction == nullptr) {
    return std::make_pair(clang_getNullCursor(), clang_getNullLocation());
  }

  std::string sourceFile = instruction->getModule()->getSourceFileName();

  CXTranslationUnit unit = translationUnit(address.directory, sourceFile);

  if (unit == nullptr) {
    return std::make_pair(clang_getNullCursor(), clang_getNullLocation());
  }

  CXFile file = clang_getFile(unit, address.filepath.c_str());
  if (file == nullptr) {
    Logger::error() << "Cannot get file from TU: " << address.filepath << "\n";
    return std::make_pair(clang_getNullCursor(), clang_getNullLocation());
  }

  CXSourceLocation location = clang_getLocation(unit, file, address.line, address.column);

  return std::make_pair(clang_getCursor(unit, location), location);
}

bool CXXJunkDetector::isJunk(MutationPoint *point) {
  auto address = getAddress(point);
  if (!address.valid()) {
    return true;
  }

  auto pair = cursorAndLocation(address, point);
  auto cursor = pair.first;
  auto location = pair.second;

  if (clang_Cursor_isNull(cursor)) {
    return true;
  }

  switch (point->getMutator()->mutatorKind()) {
    case MutatorKind::ConditionalsBoundaryMutator:
      return isJunkBoundary(cursor, location, address, point);
      break;
    case MutatorKind::MathAddMutator:
      return isJunkMathAdd(cursor, location, address, point);
      break;
    case MutatorKind::NegateMutator:
      return isJunkNegate(cursor, location, address, point);
      break;
    case MutatorKind::RemoveVoidFunctionMutator:
      return isJunkRemoveVoid(cursor, location, address, point);
      break;

    default:
      Logger::warn()
        << "CXXJunkDetector does not support '"
        << point->getMutator()->uniqueID()
        << "'\n";
      break;
  }

  return false;
}

bool CXXJunkDetector::isJunkBoundary(CXCursor cursor,
                                     CXSourceLocation location,
                                     PhysicalAddress &address,
                                     MutationPoint *point) {
  CXCursorKind kind = clang_getCursorKind(cursor);

  if (kind != CXCursor_BinaryOperator &&
      kind != CXCursor_OverloadedDeclRef) {
    return true;
  }

  return false;
}

bool CXXJunkDetector::isJunkMathAdd(CXCursor cursor,
                                    CXSourceLocation location,
                                    PhysicalAddress &address,
                                    MutationPoint *point) {
  CXCursorKind kind = clang_getCursorKind(cursor);

  if (kind != CXCursor_BinaryOperator &&
      kind != CXCursor_UnaryOperator &&
      kind != CXCursor_OverloadedDeclRef &&
      kind != CXCursor_CompoundAssignOperator) {
    return true;
  }

  return false;
}

bool CXXJunkDetector::isJunkNegate(CXCursor cursor,
                                    CXSourceLocation location,
                                    PhysicalAddress &address,
                                    MutationPoint *point) {
  CXCursorKind kind = clang_getCursorKind(cursor);

  if (kind != CXCursor_BinaryOperator &&
      kind != CXCursor_OverloadedDeclRef) {
    return true;
  }

  return false;
}

bool CXXJunkDetector::isJunkRemoveVoid(CXCursor cursor,
                                       CXSourceLocation location,
                                       PhysicalAddress &address,
                                       MutationPoint *point) {
  CXCursorKind kind = clang_getCursorKind(cursor);

  if (kind != CXCursor_CXXThisExpr &&
      kind != CXCursor_MemberRefExpr &&
      kind != CXCursor_NamespaceRef &&
      kind != CXCursor_OverloadedDeclRef &&
      kind != CXCursor_TypeRef) {
    return true;
  }

  return false;
}

