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


Mull ships as three kinds of packages:

- ``mull``: ``mull-runner`` and ``mull-reporter``, which work with any LLVM version
- ``mull-<N>``: the compiler plugin for LLVM ``<N>``, which pulls in ``mull``
- ``mull-extras-<N>``: optional tools for LLVM ``<N>``, which pull in ``mull``:
  ``mull-instrument-<N>`` adds mutants to LLVM bitcode, ``mull-rewriter-<N>`` writes the
  mutants of a report into the sources

Install the plugin for your LLVM version:

.. code-block:: bash

    # Ubuntu / Debian
    sudo apt-get update && apt-get install mull-19
    # Red Hat
    sudo yum update && yum install mull-21

The optional tools come the same way, e.g. ``apt-get install mull-extras-19``.

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

Install from GitHub Releases
----------------------------

The same packages are attached to every `GitHub release <https://github.com/mull-project/mull/releases>`_,
in case Cloudsmith is unavailable. Download ``mull`` (``Mull-<version>-<os>-<arch>-<os version>``)
and the plugin for your LLVM version (``Mull-<N>-<version>-LLVM-<N.x.y>-<os>-<arch>-<os version>``),
then install both files so that the package manager pulls in their dependencies:

.. code-block:: bash

    # Ubuntu 24.04, x86_64, LLVM 19
    gh release download 0.36.0 --repo mull-project/mull \
      --pattern 'Mull-0.36.0-ubuntu-amd64-24.04.deb' \
      --pattern 'Mull-19-0.36.0-*-ubuntu-amd64-24.04.deb'
    sudo apt-get install ./Mull-*.deb

    # RHEL 9.6, x86_64, LLVM 21
    gh release download 0.36.0 --repo mull-project/mull \
      --pattern 'Mull-0.36.0-rhel-amd64-9.6.rpm' \
      --pattern 'Mull-21-0.36.0-*-rhel-amd64-9.6.rpm'
    sudo dnf install ./Mull-*.rpm

Without ``gh``, download the same files from the release page. Packages installed this way don't
get updates, so repeat the steps to upgrade.

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

The optional tools are in a separate formula:

.. code-block:: bash

    $ brew install mull-project/mull/mull-extras@19
