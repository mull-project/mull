Multiple Test Targets
=====================

.. include:: ./_support.rst

Previous tutorials were focusing on a single test suite covering the whole software,
but in real world scenarios this is not usually the case.

What we typically have is a piece of software and several test targets, covering
different aspects of the said software. Both ``fmt`` and ``openssl`` test suites cover such a use case.

To handle this case Mull has a separate tool called ``mull-reporter`` which can be used for offline
analysis of results.

The usage is straghtforward: accumulate the mutation results across several runs into a single SQLite database,
and then let the reporter analyze the database and show the results.

Here is an example of how to use it with ``fmt``. The example is based on the :doc:`CMake tutorial <./CMakeIntegration>`.

.. code-block:: bash

    > mull-runner-18 --reporters SQLite --reporters IDE --report-name fmt bin/core-test
    [info] Using config /workspaces/mull/tests/end2end/mull.yml
    [info] Warm up run (threads: 1)
          [################################] 1/1. Finished in 178ms
    [info] Filter mutants (threads: 1)
          [################################] 1/1. Finished in 0ms
    [info] Baseline run (threads: 1)
          [################################] 1/1. Finished in 171ms
    [info] Running mutants (threads: 8)
          [################################] 72/72. Finished in 2050ms
    [info] Results can be found at './fmt.sqlite'
    [info] Survived mutants (67/72):
    <skipped>
    [info] Mutation score: 6%
    [info] Total execution time: 2432ms
    [info] Surviving mutants: 67

    > mull-runner-18 --reporters SQLite --reporters IDE --report-name fmt bin/chrono-test
    [info] Using config /workspaces/mull/tests/end2end/mull.yml
    [info] Warm up run (threads: 1)
          [################################] 1/1. Finished in 170ms
    [info] Filter mutants (threads: 1)
          [################################] 1/1. Finished in 0ms
    [info] Baseline run (threads: 1)
          [################################] 1/1. Finished in 172ms
    [info] Running mutants (threads: 8)
          [################################] 72/72. Finished in 2177ms
    [info] Results can be found at './fmt.sqlite'
    [info] Survived mutants (63/72):
    <skipped>
    [info] Mutation score: 12%
    [info] Total execution time: 2545ms
    [info] Surviving mutants: 63


These commands run mutation analysis against two different targets, each reporting the number of survived mutants.
The problem is, however, that some mutants are duplicated across both targets, and some mutants survived for ``bin/core-test``
might have been killed by the ``bin/chrono-test`` test suite, so it does make sense to analyze these two targets together.

To do so we will feed the combined results from ``fmt.sqlite`` (coming from the ``--report-name fmt`` CLI option) into the
``mull-reporter`` tool.

.. code-block:: bash

    > mull-reporter-18 --reporters IDE ./fmt.sqlite
    [info] Using config /workspaces/mull/tests/end2end/mull.yml
    [info] Survived mutants (62/72):
    <skipped>
    [info] Mutation score: 13%
    [info] Total reporting time: 18ms
    [info] Surviving mutants: 62


Overall, these two targets combined show that the actual mutation score of ``fmt`` test suite is actually
higher than when analyzed individually.

For the example of the full score, consider the other ``fmt``-based tutorial: :doc:`CTest integration <./CTestIntegration>`
