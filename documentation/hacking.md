# Hacking on Mull

## Internals

Before you start hacking it may be helpful to get through the seconds and third
sections of this paper: https://lowlevelbits.org/pdfs/Mull_Mutation_2018.pdf

## Development Setup using Vagrant

Mull supplies a number of ready to use virtual machines based on [VirtualBox](http://virtualbox.org/).

The machines are managed using [Vagrant](https://www.vagrantup.com) and [Ansible](https://www.ansible.com).

Do the following steps to to setup a virtual machine:

```
cd infrastructure
vagrant up debian
```

This command will:

 - setup a vrtual machine
 - install required packages (cmake, sqlite3, pkg-config, ...)
 - download precompiled version of LLVM
 - build Mull against the LLVM
 - run Mull's test suite
 - run Mull against OpenSSL and fmtlib as an integration test

Once the machine is up and running you can start hacking over SSH:

```
vagrant ssh debian
```

Within the virtual machine Mull's sources located under `/opt/mull`.

Alternatively, you can setup a remote toolchain within your IDE, if it supports
it.

When you are done feel free to drop the virtual machine:

```
vagrant destroy debian
```

You can see the full list of supplied VMs by running this command:

```
vagrant status
```

## Local Development Setup

You can replicate all the steps managed by Vagrant/Ansible manually.

### Required packages

Please, look at the correspoding Ansible playbook (`debian-playbook.yaml`,
`macos-playbook.yaml`, etc.) for the list of packages required on your OS.

### LLVM

You need LLVM to build and debug Mull.
You can use any LLVM version between 3.9 and 8.0.

There are several options:

1. Download precompiled version of LLVM from the official website: http://releases.llvm.org/
   This is a recommended option. Use it whenever possible. Simply download the
   tarball and unpack it somewhere.

2. Build LLVM from scratch on your own
   This option also works. Use it whenever you cannot or do not want to use get precompiled version.

3. Ask Mull to build LLVM for you
   This is recommended only if you need to debug some issue in Mull that
   requires deep dive into the LLVM itself.

**If you are going for an option 2 or 3 - make sure you also include Clang.**

### Build Mull

Create a build folder and initialize build system:

```
git clone https://github.com/mull-project/mull.git
cd mull
mkdir build.dir
cd build.dir
cmake -DPATH_TO_LLVM=path/to/llvm ..
make mull-cxx
make mull-tests
```

The `PATH_TO_LLVM` depends on which option you picked in previous section:

1. Path to extracted tar ball.
2. Path to a build directory.
3. Path to a source directory.

If you are getting linker errors, then it is very likely related to the C++
ABI. Depending on your OS/setup you may need to tweak the `_GLIBCXX_USE_CXX11_ABI` (0 or 1):

```
cmake -DPATH_TO_LLVM=some-path -DCMAKE_CXX_FLAGS=-D_GLIBCXX_USE_CXX11_ABI=0 ..
```
