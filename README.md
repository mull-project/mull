# Mull

See [LLVM-based Mutation Testing System. Request For
Comments](http://lowlevelbits.org/llvm-based-mutation-testing-system/).

## Getting started

```bash
mkdir -p ~/Projects/LLVM
cd ~/Projects/LLVM

git clone http://llvm.org/git/llvm.git
cd llvm
git checkout dc30ded6fb9526aba31a86b97e8f69fc9baece00

git clone https://github.com/mull-project/mull.git projects/mull

cd ~/Projects/LLVM
mkdir Build

cd Build
cmake -G Xcode -DBUILD_SHARED_LIBS=true -DLLVM_TARGETS_TO_BUILD="X86" ../llvm

open LLVM.xcodeproj
```

When opening Xcode first time do not autocreate schemes but choose option to
create them manually otherwise you'll have a lot of LLVM-related schemes which
you don't need.

Currently the work is being done against MullUnitTests scheme so that's the
only scheme that should be created and used.

## Notes

Update to LLVM's trunk should be a recurrent task, but first we need to get
experience of living in a downstream world.

Current code is being developed against LLVM commit:
[dc30ded6fb9526aba31a86b97e8f69fc9baece00](https://github.com/llvm-mirror/llvm/commit/dc30ded6fb9526aba31a86b97e8f69fc9baece00).

## Useful Material (papers, articles, talks, etc.)

### Papers

[An Analysis and Survey of the Development of
Mutation Testing](http://www0.cs.ucl.ac.uk/staff/mharman/tse-mutation-survey.pdf)

[Are Mutants a Valid Substitute for Real Faults in Software Testing?](https://homes.cs.washington.edu/~mernst/pubs/mutation-effectiveness-fse2014.pdf)
