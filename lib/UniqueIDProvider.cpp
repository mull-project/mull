#include "UniqueIDProvider.h"

#include "ModuleLoader.h"
#include "MutationPoint.h"
#include "MutationOperators/MutationOperator.h"

#include "llvm/IR/Module.h"
#include "llvm/Support/MD5.h"

#include <string>

using namespace Mutang;
using namespace llvm;
using namespace std;

static string fileNameFromPath(const string &path) {
  std::string filename;
  auto lastSeparatorPos = path.rfind("/");
  if (lastSeparatorPos == string::npos) {
    filename = path;
  } else {
    filename = path.substr(lastSeparatorPos + 1);
  }

  auto dotPos = filename.find(".");
  if (dotPos != string::npos) {
    filename = filename.substr(0, dotPos);
  }

  return filename;
}

static string stringFromMutationPoint(const MutationPoint &point) {
  MutationPointAddress address = point.getAddress();
  string addressID = to_string(address.getFnIndex()) + "_"
    + to_string(address.getBBIndex()) + "_"
    + to_string(address.getIIndex());

  string operatorID = point.getOperator()->uniqueIdentifier();

  string pointID = addressID + "_" + operatorID;

  return pointID;
}

string UniqueIDProvider::uniqueIDForModule(const MutangModule &module) {
  string moduleID   = module.getModule()->getModuleIdentifier();
  string moduleHash = module.getHash();

  string uniqueID = fileNameFromPath(moduleID) + "_" + moduleHash;

  return uniqueID;
}

std::string UniqueIDProvider::uniqueIDForModuleWithMutationPoint(
                                                   const MutangModule &module,
                                                   const MutationPoint &point) {
  string moduleID = uniqueIDForModule(module);
  string pointID = stringFromMutationPoint(point);

  string uniqueID = moduleID + "_" + pointID;

  return uniqueID;
}
