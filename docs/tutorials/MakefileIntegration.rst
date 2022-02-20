Makefile Integration: OpenSSL
=============================

This tutorial demonstrates how to integrate Mull into a custom Makefile-based build system.

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

    [info] Using config /tmp/sc-g6cD7gfN4/openssl/mull.yml
    [info] Warm up run (threads: 1)
           [################################] 1/1. Finished in 638ms
    [info] Baseline run (threads: 1)
           [################################] 1/1. Finished in 281ms
    [info] Running mutants (threads: 8)
           [################################] 1606/1606. Finished in 147786ms
    [info] Survived mutants (1588/1606):
    /tmp/sc-g6cD7gfN4/openssl/apps/lib/opt.c:1126:15: warning: Survived: Replaced + with - [cxx_add_to_sub]
            i = 2 + (int)strlen(o->name);
                  ^
    /tmp/sc-g6cD7gfN4/openssl/apps/lib/opt.c:1128:20: warning: Survived: Replaced + with - [cxx_add_to_sub]
                i += 1 + strlen(valtype2param(o));
                       ^
    /tmp/sc-g6cD7gfN4/openssl/crypto/aria/aria.c:546:20: warning: Survived: Replaced + with - [cxx_add_to_sub]
        int Nr = (bits + 256) / 32;
                       ^
    /tmp/sc-g6cD7gfN4/openssl/crypto/asn1/a_bitstr.c:62:13: warning: Survived: Replaced + with - [cxx_add_to_sub]
        ret = 1 + len;
                ^
    <truncated>
    /tmp/sc-g6cD7gfN4/openssl/test/testutil/format_output.c:282:47: warning: Survived: Replaced + with - [cxx_add_to_sub]
        l1 = bn1 == NULL ? 0 : (BN_num_bytes(bn1) + (BN_is_negative(bn1) ? 1 : 0));
                                                  ^
    /tmp/sc-g6cD7gfN4/openssl/test/testutil/format_output.c:283:47: warning: Survived: Replaced + with - [cxx_add_to_sub]
        l2 = bn2 == NULL ? 0 : (BN_num_bytes(bn2) + (BN_is_negative(bn2) ? 1 : 0));
                                                  ^
    /tmp/sc-g6cD7gfN4/openssl/test/testutil/format_output.c:301:32: warning: Survived: Replaced + with - [cxx_add_to_sub]
        len = ((l1 > l2 ? l1 : l2) + bytes - 1) / bytes * bytes;
                                   ^
    /tmp/sc-g6cD7gfN4/openssl/test/testutil/random.c:24:54: warning: Survived: Replaced + with - [cxx_add_to_sub]
        test_random_state[pos] += test_random_state[(pos + 28) % 31];
                                                         ^
    [info] Mutation score: 1%
    [info] Total execution time: 149344ms

Mull says that 1588 out of 1606 mutants survived. That's a lot.
Why so many and how do we handle this?

The answer is in the next tutorial :doc:`Keeping mutants under control <./ControlMutationsTutorial>`.
