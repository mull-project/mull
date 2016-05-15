# Mutang-project

## Getting started

```bash
mkdir ~/Projects/LLVM
cd ~/Projects/LLVM

git clone http://llvm.org/git/llvm.git
git clone http://llvm.org/git/compiler-rt.git llvm/projects/compiler-rt

git clone git@bitbucket.org:AlexDenisov/mutang.git llvm/projects/mutang-project

mkdir Build

cd Build
cmake -G Xcode -DBUILD_SHARED_LIBS=true -DLLVM_TARGETS_TO_BUILD="X86" ../llvm
```

