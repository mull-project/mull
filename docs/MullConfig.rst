Configuring Mull
=================

Mull's IR frontend is configured via a text file in the ``yaml`` format.

By default, Mull is looking for ``mull.yml`` file in the current directory.
If it cannot find it, then it tries the parent directory and does so recursively
until it finds the config file or reaches the root of the file system.

Alternatively, you can set ``MULL_CONFIG`` to point to the config file.

Here is an example config file:

.. code-block:: yaml

    mutators:
     - cxx_add_to_sub
     - cxx_logical
    excludePaths: # support regex
     - gtest
     - gmock
    timeout: # milliseconds
     - 10000 # 10 seconds
    quiet: false # enables additional logging
