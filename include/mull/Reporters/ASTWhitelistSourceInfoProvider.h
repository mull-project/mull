//
// Created by Stanislav Pankevich on 2019-10-18.
//

#pragma once

#include "mull/AST/ASTMutations.h"
#include "mull/Reporters/ASTSourceInfoProvider.h"

namespace mull {
class ASTWhitelistSourceInfoProvider : public SourceInfoProvider {
public:
  ~ASTWhitelistSourceInfoProvider() = default;
  explicit ASTWhitelistSourceInfoProvider(mull::ASTStorage &astStorage,
                                          mull::ASTMutations &mutations);
  MutationPointSourceInfo getSourceInfo(MutationPoint *mutationPoint) override;

private:
  ASTStorage &astStorage;
  ASTMutations &mutations;
};
} // namespace mull
