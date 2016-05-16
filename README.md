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

