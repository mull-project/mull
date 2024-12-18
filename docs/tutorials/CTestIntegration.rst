CTest Integration
=================

.. include:: ./_support.rst

At the moment, Mull doesn't support CTest natively, but it's possible to make it work with
shell-scripting and the support of :doc:`multiple test targets <./MultipleTestTargets>`.

We are again using ``fmt`` as an example here. It's possible to run the whole test suite using ``CTest``:

.. code-block:: bash

    > ctest
    Test project /workspaces/mull/tests/end2end/fmt/fmt.build.dir
          Start  1: assert-test
    1/12 Test  #1: assert-test ......................   Passed    0.01 sec
          Start  2: chrono-test
    2/12 Test  #2: chrono-test ......................   Passed    0.01 sec
          Start  3: core-test
    3/12 Test  #3: core-test ........................   Passed    0.01 sec
          Start  4: gtest-extra-test
    4/12 Test  #4: gtest-extra-test .................   Passed    0.01 sec
          Start  5: format-test
    5/12 Test  #5: format-test ......................   Passed    0.03 sec
          Start  6: format-impl-test
    6/12 Test  #6: format-impl-test .................   Passed    0.01 sec
          Start  7: locale-test
    7/12 Test  #7: locale-test ......................   Passed    0.01 sec
          Start  8: ostream-test
    8/12 Test  #8: ostream-test .....................   Passed    0.01 sec
          Start  9: printf-test
    9/12 Test  #9: printf-test ......................   Passed    0.01 sec
          Start 10: time-test
    10/12 Test #10: time-test ........................   Passed    0.01 sec
          Start 11: custom-formatter-test
    11/12 Test #11: custom-formatter-test ............   Passed    0.01 sec
          Start 12: ranges-test
    12/12 Test #12: ranges-test ......................   Passed    0.01 sec

    100% tests passed, 0 tests failed out of 12

    Total Test time (real) =   0.17 sec

However, Mull is not involved in this test run, CTest just runs the normal test suite.

To make it work, you have to run Mull against each test target explicitly.

In case of ``fmt``, you can do it as follows. The snippet relies on the knowledge of the project layout, so you'd need to adjust it for your project.


.. code-block:: bash

    for test_case in $(ctest --show-only | grep 'Test .*:' | awk ' { print $3} ');
    do
      echo "Testing $test_case"
      mull-runner-12 --allow-surviving --reporters SQLite --report-name fmt ./bin/$test_case
    done

After all the runs are completed, you can use ``mull-reporter`` to analyze :doc:`multiple test targets <./MultipleTestTargets>`:

.. code-block:: bash

    > mull-reporter-12 --reporters IDE ./fmt.sqlite
    [info] Using config /workspaces/mull/tests/end2end/mull.yml
    [info] Survived mutants (47/100):
    <skipped>
    [info] Mutation score: 53%
    [info] Total reporting time: 22ms
    [info] Surviving mutants: 47
