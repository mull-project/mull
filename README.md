# Mutang-project

See [LLVM-based Mutation Testing System. Request For Comments](http://lowlevelbits.org/llvm-based-mutation-testing-system/).

## Getting started

```bash
mkdir -p ~/Projects/LLVM
cd ~/Projects/LLVM

git clone http://llvm.org/git/llvm.git

git clone git@github.com:AlexDenisov/mutang.git llvm/projects/mutang-project

mkdir Build

cd Build
cmake -G Xcode -DBUILD_SHARED_LIBS=true -DLLVM_TARGETS_TO_BUILD="X86" ../llvm

open LLVM.xcodeproj
```

When opening Xcode first time do not autocreate schemes but choose option to create them manually otherwise you'll have a lot of LLVM-related schemes which you don't need.

Currently the work is being done against MutangUnitTests scheme so that's the only scheme that should be created and used.

## Notes

Update to LLVM's trunk should be a recurrent task, but first we need to get experience of living in a downstream world.

Current code is being developed against LLVM commit: [d9176f9ab2d183a1cf2b2226cf3943eec6b16c6a](https://github.com/llvm-mirror/llvm/commit/d9176f9ab2d183a1cf2b2226cf3943eec6b16c6a).

