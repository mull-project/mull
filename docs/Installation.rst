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

*Note*: Mull uses `Cloudsmith <https://cloudsmith.io>`_ for package distribution.
The above script detects your OS and sets up the apt repo automagically.

Install the package:

.. code-block:: bash

    sudo apt-get update
    sudo apt-get install mull-10 # Ubuntu 18.04
    sudo apt-get install mull-12 # Ubuntu 20.04

Check if everything works:

.. code-block:: bash

    $ mull-runner-10 --version
    Mull: Practical mutation testing for C and C++
    Home: https://github.com/mull-project/mull
    Docs: https://mull.readthedocs.io
    Version: 0.15.0
    Commit: ab159cd
    Date: 20 Jan 2022
    LLVM: 10.0.0

You can also get the latest "nightly" build using the corresponding source:

.. code-block:: bash

    curl -1sLf 'https://dl.cloudsmith.io/public/mull-project/mull-nightly/setup.deb.sh' | sudo -E bash

Links:

  - `Mull Stable <https://cloudsmith.io/~mull-project/repos/mull-stable/packages/>`_
  - `Mull Nightly <https://cloudsmith.io/~mull-project/repos/mull-stable/packages/>`_

Install on macOS
****************

Download the latest version from `Github Releases <https://github.com/mull-project/mull/releases/latest>`_.

Check the installation:

.. code-block:: bash

    $ mull-runner-13 --version
    Mull: Practical mutation testing for C and C++
    Home: https://github.com/mull-project/mull
    Docs: https://mull.readthedocs.io
    Version: 0.15.0
    Commit: 0252a4cf
    Date: 28 Jan 2022
    LLVM: 13.0.0

You can also get the latest "nightly" build from `here <https://cloudsmith.io/~mull-project/repos/mull-nightly/packages/>`_.
