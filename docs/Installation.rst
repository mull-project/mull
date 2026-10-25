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

    # Ubuntu / Debian
    curl -1sLf 'https://dl.cloudsmith.io/public/mull-project/mull-stable/setup.deb.sh' | sudo -E bash
    # Red Hat
    curl -1sLf 'https://dl.cloudsmith.io/public/mull-project/mull-stable/setup.rpm.sh' | sudo -E bash


Mull ships as two kinds of packages:

- ``mull``: ``mull-runner`` and ``mull-reporter``, which work with any LLVM version
- ``mull-<N>``: the compiler plugin and ``mull-instrument`` for LLVM ``<N>``, which pull in ``mull``

Install the plugin for your LLVM version:

.. code-block:: bash

    # Ubuntu / Debian
    sudo apt-get update && apt-get install mull-19
    # Red Hat
    sudo yum update && yum install mull-21

Check if everything works:

.. code-block:: bash

    $ mull-runner --version
    mull-runner 0.36.0

    Mull: Practical mutation testing and fault injection for C and C++
    Home: https://mull-project.com
    Plugins:
      /usr/lib/mull-ir-frontend-19 (mull 0.36.0, LLVM 19.1.7)

.. note::

    Before 0.36.0 the tools had versioned names (``mull-runner-19``). They are still installed
    as symlinks to ``mull-runner`` and ``mull-reporter``, but will be removed in a future release.

Install on macOS
****************

Use Homebrew, or download the latest version from `GitHub Releases <https://github.com/mull-project/mull/releases/latest>`_.

.. code-block:: bash

    $ brew install mull-project/mull/mull@19
    $ mull-runner --version
    mull-runner 0.36.0

    Mull: Practical mutation testing and fault injection for C and C++
    Home: https://mull-project.com
    Plugins:
      /opt/homebrew/lib/mull-ir-frontend-19 (mull 0.36.0, LLVM 19.1.7)
