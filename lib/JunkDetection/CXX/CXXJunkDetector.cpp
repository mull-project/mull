#include "JunkDetection/CXX/CXXJunkDetector.h"

#include "MutationPoint.h"
#include "MutationOperators/MutationOperator.h"
#include "Logger.h"

#include <llvm/IR/DebugInfoMetadata.h>
#include <llvm/IR/DebugLoc.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Instruction.h>

#include <llvm/Support/Path.h>

using namespace mull;
using namespace llvm;

#pragma mark - Phisycal Address

PhysicalAddress getAddress(MutationPoint *point) {
  PhysicalAddress address;

  if (auto instruction = dyn_cast<Instruction>(point->getOriginalValue())) {
    if (instruction->getMetadata(0)) {
      auto debugInfo = instruction->getDebugLoc();

      auto file = debugInfo->getFilename().str();
      if (sys::path::is_absolute(file)) {
        address.filepath = file;
      } else {
        auto directory = debugInfo->getDirectory().str();
        address.filepath = directory + sys::path::get_separator().str() + file;
      }

      address.line = debugInfo->getLine();
      address.column = debugInfo->getColumn();
    }
  }

  return address;
}

PhysicalAddress::PhysicalAddress() : filepath(""), line(0), column(0) {}

bool PhysicalAddress::valid() {
  if (filepath != "" && line != 0 && column != 0) {
    return true;
  }

  return false;
}

#pragma mark - Junk Detector

CXXJunkDetector::CXXJunkDetector() : index(clang_createIndex(true, true)) {}

CXXJunkDetector::~CXXJunkDetector() {
  for (auto &pair : units) {
    clang_disposeTranslationUnit(pair.second);
  }
  clang_disposeIndex(index);
}

std::pair<CXCursor, CXSourceLocation>
CXXJunkDetector::cursorAndLocation(PhysicalAddress &address) {
  if (units.count(address.filepath) == 0) {
    const char *argv[] = { nullptr };
    const int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    CXTranslationUnit unit = clang_parseTranslationUnit(index,
                                                        address.filepath.c_str(),
                                                        argv, argc,
                                                        nullptr, 0,
                                                        CXTranslationUnit_KeepGoing);
    if (unit == nullptr) {
      Logger::error() << "Cannot parse translation unit: " << address.filepath << "\n";
      return std::make_pair(clang_getNullCursor(), clang_getNullLocation());
    }
    units[address.filepath] = unit;
  }

  CXTranslationUnit unit = units[address.filepath];
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

  auto pair = cursorAndLocation(address);
  auto cursor = pair.first;
  auto location = pair.second;

  if (clang_Cursor_isNull(cursor)) {
    return true;
  }

  switch (point->getOperator()->mutatorKind()) {
    case MutatorKind::CXXBoundaryMutator:
      return isJunkBoundary(cursor, location, address, point);
      break;

    default:
      Logger::warn()
        << "CXXJunkDetector does not support '"
        << point->getOperator()->uniqueID()
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

  if (kind != CXCursor_BinaryOperator) {
    return true;
  }

  return false;
}
