Hacking On Mull
===============

Internals
*********

Before you start hacking it may be helpful to get through the second and third sections of this paper:
`Mull it over: mutation testing based on LLVM <https://lowlevelbits.org/pdfs/Mull_Mutation_2018.pdf>`_
from `ICST 2018 <https://www.es.mdh.se/icst2018/>`_.

Development Setup using Vagrant
*******************************

Mull supplies a number of ready to use virtual machines based on `VirtualBox <http://virtualbox.org>`_.

The machines are managed using `Vagrant <https://www.vagrantup.com>`_ and `Ansible <https://www.ansible.com>`_.

Do the following steps to setup a virtual machine:

.. code-block:: bash

    cd infrastructure
    vagrant up debian

This command will:

- setup a virtual machine
- install required packages (cmake, sqlite3, pkg-config, ...)
- download precompiled version of LLVM
- build Mull against the LLVM
- run Mull's test suite
- run Mull against OpenSSL and fmtlib as an integration test

Once the machine is up and running you can start hacking over SSH:

.. code-block:: bash

    vagrant ssh debian

Within the virtual machine Mull's sources located under ``/opt/mull``.

Alternatively, you can setup a remote toolchain within your IDE, if it supports
it.

When you are done feel free to drop the virtual machine:

.. code-block:: bash

    vagrant destroy debian

You can see the full list of supplied VMs by running this command:

.. code-block:: bash

    vagrant status

Local Development Setup
***********************

You can replicate all the steps managed by Vagrant/Ansible manually.

Required packages
-----------------

Please, look at the corresponding `Ansible playbook <https://github.com/mull-project/mull/tree/main/infrastructure>`_
(``debian-playbook.yaml``, ``macos-playbook.yaml``, etc.) for the list of packages required on your OS.

LLVM
----

You need LLVM to build and debug Mull.
You can use any LLVM version between 9.0 and 13.0.

As of the version 0.14.0, Mull can be compiled against LLVM/Clang available
through your package manager (e.g. apt or homebrew).

Build Mull
----------

Create a build folder and initialize build system:

.. code-block:: bash

    git clone https://github.com/mull-project/mull.git --recursive
    cd mull
    mkdir build.dir
    cd build.dir
    cmake -DCMAKE_PREFIX_PATH=<cmake search paths> ..
    make mull-runner-12
    make mull-tests

The ``cmake search paths`` should point to the LLVM/Clang CMake config folders.
Some examples:

 - llvm\@12 installed via homebrew on macOS: ``"/usr/local/opt/llvm@12/lib/cmake/llvm/;/usr/local/opt/llvm@12/lib/cmake/clang/"``
 - llvm-12 installed via apt on Ubuntu: ``"/usr/lib/llvm-13/cmake/;/usr/lib/cmake/clang-13/"``

If you are getting linker errors, then it is very likely related to the C++
ABI. Depending on your OS/setup you may need to tweak the ``_GLIBCXX_USE_CXX11_ABI`` (0 or 1):

.. code-block:: bash

    cmake -DCMAKE_PREFIX_PATH=<cmake search paths> -DCMAKE_CXX_FLAGS=-D_GLIBCXX_USE_CXX11_ABI=0 ..
