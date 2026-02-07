Configuring Mull
=================

.. include:: ./_support.rst

Mull's IR frontend is configured via a text file in the ``yaml`` format.

By default, Mull is looking for ``mull.yml`` file in the current directory.
If it cannot find it, then it tries the parent directory and does so recursively
until it finds the config file or reaches the root of the file system.

Alternatively, you can set ``MULL_CONFIG`` to point to the config file.

All mull tools (compiler plugin, runner, reporter) reuse the same config.
Config of CLI tools (runner, reporter) can be overriden via the corresponding CLI flags.

Here is an example config file containing all the supported fields.

.. include:: generated/mull-config-example.rst
