Keeping mutants under control
=============================

This tutorial shows how to control the amount of mutants.

---

When you apply mutation testing for the first time, you might be overwhelmed by
the number of mutants - what do you do when you see that several hundred or thousands of mutants survived?

The right way to go about it is to put the number of mutants under control and
work through them incrementally.

OpenSSL Example
---------------

The :doc:`OpenSSL <./MakefileIntegration>` tutorial makes a great example of when we want to decrease the amount of mutants.

To recap, recreate the same setup.

1. Checkout OpenSSL:

.. code-block:: bash

    git clone https://github.com/openssl/openssl.git \
      --branch openssl-3.0.1 \
      --depth 1

2. Create Mull config file ``openssl/mull.yml``:

.. code-block:: yaml

    mutators:
      - cxx_add_to_sub

3. Build OpenSSL:

.. code-block:: bash

    cd openssl
    export CC=clang-12
    ./config -O0 -fexperimental-new-pass-manager \
      -fpass-plugin=/usr/lib/mull-ir-frontend-12 \
      -g -grecord-command-line
    make build_generated -j
    make ./test/bio_enc_test -j

4. Run Mull:

.. code-block:: bash

    $ mull-runner-12 ./test/bio_enc_test
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


In the end, you should see about ~1.5k survived mutants.

There are at least two kinds of "problematic" mutants there:

- not interesting: e.g., we probably don't care about mutants under ``testutil``
- unreachable: the test suite cannot detect them

Let's try to fix these issues one by one.

File Path Filters
-----------------

First, let's tell Mull to not mutate and not to run Mutants under ``testutil``.

We can extend the same ``mull.yml`` file we used to configure Mull at the very beginning.

Mull comes with two path-based filters: ``excludePaths`` and ``includePaths``.
You can use these to either exclude or include mutations based on their file-system location.
To ignore any mutants under ``testutil`` edit ``mull.yml`` as follows:

.. code-block:: yaml

    mutators:
      - cxx_add_to_sub
    excludePaths:
      - .*testutil.*


Now, rerun Mull:

.. code-block:: bash

    $ mull-runner-12 ./test/bio_enc_test
    [info] Using config /tmp/sc-g6cD7gfN4/openssl/mull.yml
    [info] Warm up run (threads: 1)
           [################################] 1/1. Finished in 282ms
    [info] Filter mutants (threads: 1)
           [################################] 1/1. Finished in 2ms
    [info] Baseline run (threads: 1)
           [################################] 1/1. Finished in 283ms
    [info] Running mutants (threads: 8)
           [################################] 1585/1585. Finished in 149522ms
    [info] Survived mutants (1568/1585):
    <truncated>
    [info] Mutation score: 1%
    [info] Total execution time: 150815ms


.. note::
   Some config options understood by both ``mull-ir-frontend`` and ``mull-runner``.
   In this case, we don't need to recompile the program under test - ``mull-runner`` picks
   up the config changes and filters out not needed mutants.

   Though, ``./test/bio_enc_test`` still contains the mutants from ``testutil``, they are just ignored.

Total 1585 mutants vs 1606 previously. Slightly better, but still not great.

We need something heavier than that!

Code Coverage Filter
--------------------

Mull understands code coverage, but for that to work we should recompile OpenSSL
to include the instrumentation information:

.. code-block:: bash

    make clean
    ./config -O0 -fexperimental-new-pass-manager \
      -fpass-plugin=/usr/lib/mull-ir-frontend-12 \
      -g -grecord-command-line \
      -fprofile-instr-generate -fcoverage-mapping
    make build_generated -j
    make ./test/bio_enc_test -j

.. note::
   This time, ``mull-ir-frontend`` picks up ``excludePaths`` from ``mull.yml``
   and ``./test/bio_enc_test`` no longer contains mutations from ``testutil``.


Rerun Mull:

.. code-block:: bash

    $ mull-runner-12 ./test/bio_enc_test
    [info] Using config /tmp/sc-g6cD7gfN4/openssl/mull.yml
    [info] Warm up run (threads: 1)
           [################################] 1/1. Finished in 1281ms
    [info] Extracting coverage information (threads: 1)
           [################################] 1/1. Finished in 361ms
    [info] Filter mutants (threads: 1)
           [################################] 1/1. Finished in 36ms
    [info] Baseline run (threads: 1)
           [################################] 1/1. Finished in 326ms
    [info] Running mutants (threads: 8)
           [################################] 34/34. Finished in 7805ms
    [info] Survived mutants (18/34):
    /tmp/sc-g6cD7gfN4/openssl/crypto/conf/conf_mod.c:556:22: warning: Survived: Replaced + with - [cxx_add_to_sub]
        size = strlen(t) + strlen(sep) + strlen(OPENSSL_CONF) + 1;
                         ^
    /tmp/sc-g6cD7gfN4/openssl/crypto/conf/conf_mod.c:556:36: warning: Survived: Replaced + with - [cxx_add_to_sub]
        size = strlen(t) + strlen(sep) + strlen(OPENSSL_CONF) + 1;
                                       ^
    <truncated>
    /tmp/sc-g6cD7gfN4/openssl/providers/implementations/rands/drbg_ctr.c:427:37: warning: Survived: Replaced + with - [cxx_add_to_sub]
            ctr32 = GETU32(ctr->V + 12) + blocks;
                                        ^
    /tmp/sc-g6cD7gfN4/openssl/providers/implementations/rands/drbg_ctr.c:555:28: warning: Survived: Replaced + with - [cxx_add_to_sub]
        drbg->seedlen = keylen + 16;
                               ^
    /tmp/sc-g6cD7gfN4/openssl/providers/implementations/rands/seed_src.c:191:44: warning: Survived: Replaced + with - [cxx_add_to_sub]
        bytes_needed = entropy >= 0 ? (entropy + 7) / 8 : 0;
                                               ^
    [info] Mutation score: 47%
    [info] Total execution time: 12449ms

A few things worth mentioning here:

- there is a new running phase ``Extracting coverage information``: Mull handles code coverage info automatically
- we've got 34 mutants instead of ~1.5k
- total execution time dropped from ~150 seconds to only 12 seconds

With this improvement in place there are two ways forward:

1. Extend the test suite to ensure there are no survived mutants
2. Add more :doc:`mutators <../SupportedMutations>` and go to the step 1 above.
