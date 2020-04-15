Installation
============

Mull comes with a number of precompiled binaries for macOS and Ubuntu.

Please, refer to the `Hacking on Mull <HackingOnMull.html>`_ to build Mull from sources.

Install on Ubuntu
*****************

.. code-block:: bash

    wget https://api.bintray.com/users/bintray/keys/gpg/public.key
    sudo apt-key add public.key
    echo "deb https://dl.bintray.com/mull-project/ubuntu-18 stable main" | sudo tee -a /etc/apt/sources.list
    sudo apt-get update
    sudo apt-get install mull

Check if everything works:

.. code-block:: bash

    $ mull-cxx --version
    Mull: LLVM-based mutation testing
    https://github.com/mull-project/mull
    Version: 0.7.0
    Commit: 1638698
    Date: 28 Mar 2020
    LLVM: 9.0.0

You can also install Mull for Ubuntu-16.04 or get the latest "nightly" build from the corresponding repositories:

.. code-block::

    deb https://dl.bintray.com/mull-project/ubuntu-16 stable main
    deb https://dl.bintray.com/mull-project/ubuntu-18 nightly main
    deb https://dl.bintray.com/mull-project/ubuntu-16 nightly main

Install on macOS
****************

Get the latest version here `Bintray <https://bintray.com/mull-project/macos/mull/_latestVersion>`_.

Or install via Homebrew:

.. code-block:: bash

    brew install mull-project/mull/mull-stable

Check the installation:

.. code-block:: bash

    $ mull-cxx --version
    Mull: LLVM-based mutation testing
    https://github.com/mull-project/mull
    Version: 0.7.0
    Commit: 1638698
    Date: 28 Mar 2020
    LLVM: 9.0.0

You can also get the latest "nightly" build from `here <https://bintray.com/mull-project/macos/mull-nightly/_latestVersion>`_.