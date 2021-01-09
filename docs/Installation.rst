Installation
============

Mull comes with a number of precompiled binaries for macOS and Ubuntu.

Please, refer to the `Hacking on Mull <HackingOnMull.html>`_ to build Mull from sources.

Install on Ubuntu
*****************

.. code-block:: bash

    wget https://api.bintray.com/users/bintray/keys/gpg/public.key
    sudo apt-key add public.key
    echo "deb https://dl.bintray.com/mull-project/ubuntu-20 stable main" | sudo tee -a /etc/apt/sources.list
    sudo apt-get update
    sudo apt-get install mull

Check if everything works:

.. code-block:: bash

    $ mull-cxx --version
    Mull: LLVM-based mutation testing
    https://github.com/mull-project/mull
    Version: 0.9.0
    Commit: 9f2d43c
    Date: 07 Jan 2021
    LLVM: 11.0.0

You can also get the latest "nightly" build from the corresponding repository:

.. code-block::

    deb https://dl.bintray.com/mull-project/ubuntu-20 nightly main

For Ubuntu 18 use one of the following repositories:

.. code-block::

    deb https://dl.bintray.com/mull-project/ubuntu-18 stable main
    deb https://dl.bintray.com/mull-project/ubuntu-18 nightly main

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
    Version: 0.9.0
    Commit: 9f2d43c
    Date: 07 Jan 2020
    LLVM: 11.0.0

You can also get the latest "nightly" build from `here <https://bintray.com/mull-project/macos/mull-nightly/_latestVersion>`_.
