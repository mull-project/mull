Installation
============

Mull comes with a number of precompiled binaries for macOS and Ubuntu.
There are two flavors of packages:

 - `stable <https://cloudsmith.io/~mull-project/repos/mull-stable/packages/>`_ - tagged releases (0.8.0, 0.9.0, etc.)
 - `nightly <https://cloudsmith.io/~mull-project/repos/mull-nightly/packages/>`_ - built for each PR

Alternatively, you can find packages on `Github <https://github.com/mull-project/mull/releases>`_.

Please, refer to the `Hacking on Mull <HackingOnMull.html>`_ to build Mull from sources.

Install on Ubuntu
*****************

Mull supports Ubuntu 18.04 and 20.04.

Setup apt-repository:

.. code-block:: bash

    curl -1sLf 'https://dl.cloudsmith.io/public/mull-project/mull-stable/setup.deb.sh' | sudo -E bash

_Note: Mull uses [Cloudsmith](https://cloudsmith.io) for package distribution.
The above script detects your OS and sets up the apt repo automagically._

Install the package:

.. code-block:: bash

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

You can also get the latest "nightly" build using the corresponding source:

.. code-block:: bash

    curl -1sLf 'https://dl.cloudsmith.io/public/mull-project/mull-nightly/setup.deb.sh' | sudo -E bash

Install on macOS
****************

Get the latest version here `Github Releases <https://github.com/mull-project/mull/releases/latest>`_.

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

You can also get the latest "nightly" build from `here <https://cloudsmith.io/~mull-project/repos/mull-nightly/packages/>`_.
