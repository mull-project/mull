Installation
============

.. include:: ./_support.rst

Precompiled packages (`arm64` and `x86_64`) are available on `Cloudsmith <https://cloudsmith.io/~mull-project/repos/mull-stable/packages/>`_
and on `GitHub <https://github.com/mull-project/mull/releases>`_.

Please, refer to the `Hacking on Mull <HackingOnMull.html>`_ to build Mull from sources.

Install on Linux
****************

The following examples setup mull repos on deb- and rpm-based systems, respectively:

.. code-block:: bash

    # Ubuntu
    curl -1sLf 'https://dl.cloudsmith.io/public/mull-project/mull-stable/setup.deb.sh' | sudo -E bash
    # Red Hat
    curl -1sLf 'https://dl.cloudsmith.io/public/mull-project/mull-stable/setup.rpm.sh' | sudo -E bash


Install the package:

.. code-block:: bash

    # Ubuntu
    sudo apt-get update && apt-get install mull-19
    # Red Hat
    sudo yum update && yum install mull-19

Check if everything works:

.. code-block:: bash

    $ mull-runner-19 --version
    Mull: Practical mutation testing and fault injection for C and C++
    Home: https://github.com/mull-project/mull
    Docs: https://mull.readthedocs.io
    Support: https://mull.readthedocs.io/en/latest/Support.html
    Version: 0.26.1
    LLVM: 19.1.7

Install on macOS
****************

Download the latest version from `GitHub Releases <https://github.com/mull-project/mull/releases/latest>`_.

Check the installation:

.. code-block:: bash

    $ mull-runner-19 --version
    Mull: Practical mutation testing and fault injection for C and C++
    Home: https://github.com/mull-project/mull
    Docs: https://mull.readthedocs.io
    Support: https://mull.readthedocs.io/en/latest/Support.html
    Version: 0.26.1
    LLVM: 19.1.7
