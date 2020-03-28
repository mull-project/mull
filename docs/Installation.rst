Installation
============

Mull comes with a number of precompiled binaries for macOS and Ubuntu.
Please, refer to the `Hacking on Mull <HackingOnMull.html>`_ to build Mull from sources.

Install on Ubuntu
*****************

Get the Bintray public GPG key:

.. code-block:: bash

    wget https://bintray.com/user/downloadSubjectPublicKey?username=bintray -O bintray.key
    sudo apt-key add bintray.key

Add the repository:

.. code-block:: bash

    sudo echo "deb https://dl.bintray.com/mull-project/ubuntu-18 stable main" >> /etc/apt/sources.list
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

Check the latest version on `Bintray <https://bintray.com/mull-project/macos/mull/_latestVersion>`_.

Go to the `Files` tab, download and unzip the version you need:

.. code-block:: bash

    wget "https://bintray.com/mull-project/macos/download_file?file_path=pool%2Fmain%2Fm%2FMull-0.7.0-LLVM-9.0-macOS-10.15.3.zip" -O mull.zip
    unzip mull.zip
    cp Mull-0.7.0-LLVM-9.0-macOS-10.15.3/bin/mull-cxx /usr/local/bin/mull-cxx

Check the installation:

.. code-block:: bash

    $ mull-cxx --version
    Mull: LLVM-based mutation testing
    https://github.com/mull-project/mull
    Version: 0.7.0
    Commit: 1638698
    Date: 28 Mar 2020
    LLVM: 9.0.0

Installation via `Homebrew <https://brew.sh>`_ is on our TODO-list.

You can also get the latest "nightly" build from `here <https://bintray.com/mull-project/macos/mull-nightly/_latestVersion>`_.