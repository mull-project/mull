Makefile Integration: OpenSSL
=============================

This tutorial demonstrates how integrate Mull into a custom Makefile-based build system.

We use OpenSSL as an example.

.. note::
   If you are new to Mull, then :doc:`Hello World example <./HelloWorld>` is a good starting point.

Step 1. Check out the source code
---------------------------------

``openssl-3.0.1`` is the latest version we tested.

.. code-block:: bash

    git clone https://github.com/openssl/openssl.git \
      --branch openssl-3.0.1 \
      --depth 1

Step 2. Create sample Mull config
---------------------------------

Create a file ``openssl/mull.yml`` with the following contents:

.. code-block:: yaml

    mutators:
      - cxx_add_to_sub


Step 3. Configure and build OpenSSL
-----------------------------------

.. code-block:: bash

    cd openssl
    export CC=clang-12
    ./config -O0 -fexperimental-new-pass-manager \
      -fpass-plugin=/usr/lib/mull-ir-frontend-12 \
      -g -grecord-command-line
    make build_generated -j
    make ./test/bio_enc_test -j


Step 4. Run Mull against OpenSSL's tests
----------------------------------------

.. code-block:: bash

    mull-runner-12 ./test/bio_enc_test

You should see similar (and pretty long) output:

.. code-block:: bash

    [info] Warm up run (threads: 1)
           [################################] 1/1. Finished in 276ms
    [info] Baseline run (threads: 1)
           [################################] 1/1. Finished in 273ms
    [info] Running mutants (threads: 8)
           [################################] 1605/1605. Finished in 155543ms
    [info] Survived mutants (1587/1605):
    /private/tmp/sc-yOzi9P1sJ/openssl/apps/lib/opt.c:1126:15: warning: Survived: Replaced + with - [cxx_add_to_sub]
            i = 2 + (int)strlen(o->name);
                  ^
    /private/tmp/sc-yOzi9P1sJ/openssl/apps/lib/opt.c:1128:20: warning: Survived: Replaced + with - [cxx_add_to_sub]
                i += 1 + strlen(valtype2param(o));
                       ^
    /private/tmp/sc-yOzi9P1sJ/openssl/crypto/aria/aria.c:546:20: warning: Survived: Replaced + with - [cxx_add_to_sub]
        int Nr = (bits + 256) / 32;
                       ^
    <truncated>
    /private/tmp/sc-yOzi9P1sJ/openssl/test/testutil/driver.c:443:54: warning: Survived: Replaced + with - [cxx_add_to_sub]
        size_t len = dirlen + strlen(sep) + strlen(file) + 1;
                                                         ^
    /private/tmp/sc-yOzi9P1sJ/openssl/test/testutil/format_output.c:282:47: warning: Survived: Replaced + with - [cxx_add_to_sub]
        l1 = bn1 == NULL ? 0 : (BN_num_bytes(bn1) + (BN_is_negative(bn1) ? 1 : 0));
                                                  ^
    /private/tmp/sc-yOzi9P1sJ/openssl/test/testutil/format_output.c:283:47: warning: Survived: Replaced + with - [cxx_add_to_sub]
        l2 = bn2 == NULL ? 0 : (BN_num_bytes(bn2) + (BN_is_negative(bn2) ? 1 : 0));
                                                  ^
    /private/tmp/sc-yOzi9P1sJ/openssl/test/testutil/format_output.c:301:32: warning: Survived: Replaced + with - [cxx_add_to_sub]
        len = ((l1 > l2 ? l1 : l2) + bytes - 1) / bytes * bytes;
                                   ^
    /private/tmp/sc-yOzi9P1sJ/openssl/test/testutil/random.c:24:54: warning: Survived: Replaced + with - [cxx_add_to_sub]
        test_random_state[pos] += test_random_state[(pos + 28) % 31];
                                                         ^
    [info] Mutation score: 1%
    [info] Total execution time: 156720ms

Next Steps
----------

Mull says that 1587 out of 1605 mutants survived. That's a lot.

There are two reasons behind such a big number:

- some mutations are unreachable: the test suite cannot detect them
- some mutations are not interesting: e.g., we probably don't care about mutants under ``testutil``

Follow this tutorial to learn how to control the amount of mutations: :doc:`Keeping mutants under control <./ControlMutationsTutorial>`
