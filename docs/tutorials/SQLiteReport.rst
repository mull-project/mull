Working with SQLite report
==========================

From the very beginning, we didn't want to impose our vision on treating the
results of mutation testing. Some people do not care about the mutation score,
while others do care, but want to calculate it slightly differently.

To solve this problem, Mull splits execution and reporting into separate phases.
What Mull does is apply mutation testing on a program, collect as much information
as possible, and then pass this information to one of several reporters.

At the moment of writing, there are three reporters:

- ``IDEReporter``: prints mutants in the format of clang warnings
- ``MutationTestingElementsReporter``: emits a JSON-file compatible with `Mutation Testing Elements <https://github.com/stryker-mutator/mutation-testing-elements>`_.
- ``SQLiteReporter``: saves all the information to an SQLite database
- ``PatchesReporter``: saves each mutant as a ``.patch`` that can be applied on the original code

One of the ways to do a custom analysis of mutation testing results is to run queries
against the SQLite database. The rest of this document describes how to work with Mull's SQLite database.

Preparing the database
----------------------

The benefit of having results in an SQLite database is that we can run as many
queries as we want and to examine the results without re-running Mull, which can
be a long-running task.

If you don't have a sample project ready, then it is a good idea to check out
the :doc:`fmtlib tutorial <./CMakeIntegration>`.

To enable SQLite reporter, add ``-reporters=SQLite`` to the CLI options.
It is also recommended to specify the report name via ``-report-name``, e.g.:

.. code-block::

     mull-runner-12 --reporters=SQLite --report-name=tutorial ./bin/core-test

In the end, you should see something like this:

.. code-block::

    [info] Results can be found at './tutorial.sqlite'

Open the database and enable better formatting (optional):

.. code-block::

    sqlite3 ./tutorial.sqlite
    sqlite> .header on
    sqlite> .mode line

Database Schema
---------------

Now you can examine contents of the database:

.. code-block::

    sqlite> .tables
    information  mutant

    sqlite> .schema information
    CREATE TABLE information (
      key TEXT,
      value TEXT
    );
    sqlite> .schema mutant
    CREATE TABLE mutant (
      mutator TEXT,
      filename TEXT,
      directory TEXT,
      line_number INT,
      column_number INT,
      status INT,
      duration INT,
      stdout TEXT,
      stderr TEXT
    );

The database contains two tables: ``mutant`` and ``information``.

The ``information`` table stores a number of key/value pairs with certain facts about Mull:

.. code-block::

    sqlite> select * from information;
      key = LLVM Version
    value = 12.0.1

      key = Build Date
    value = 08 Mar 2022

      key = Commit
    value = 8f01ac4d

      key = Mull Version
    value = 0.16.0

      key = URL
    value = https://github.com/mull-project/mull

And the ``mutant`` table stores the name of the mutation operator, the location of the mutant,
and information about the execution of each mutant: duration, status (passed, failed, etc) and the
text from standard out and err streams.

.. code-block::

    sqlite> select * from mutant limit 1;
        mutant_id = cxx_add_to_sub:/tmp/sc-76UJhQXB4/fmt/include/fmt/core.h:822:23
          mutator = cxx_add_to_sub
         filename = /tmp/sc-76UJhQXB4/fmt/include/fmt/core.h
        directory =
      line_number = 822
    column_number = 23
           status = 1
         duration = 14
           stdout = [==========] Running 55 tests from 19 test suites.
    <truncated>
           stderr =

The ``status`` field stores a numerical value as described in the following table:

.. list-table::
   :header-rows: 1

   * - Numeric value
     - String Value
     - Description
   * - 1
     - Failed
     - test has failed (the exit code does not equal 0)
   * - 2
     - Passed
     - test has passed (the exit code equals 0)
   * - 3
     - Timedout
     - test execution took more time than expected
   * - 4
     - Crashed
     - test program was terminated
   * - 5
     - AbnormalExit
     - test program exited (some test frameworks call ``exit(1)`` when test fails)
   * - 6
     - DryRun
     - test was not run (DryRun mode enabled)
   * - 7
     - FailFast
     - mutant was killed by another test so this test run can be skipped


Basic exploration
*****************

Let's check how many mutants:

.. code-block::

    sqlite>  select count(*) from mutant;
    count(*) = 163

Let's see some stats on the execution time:

.. code-block::

    sqlite> select avg(duration), max(duration) from mutant;
    avg(duration) = 10.5276073619632
    max(duration) = 104

Let's see what's wrong with that slow run:

.. code-block::

    sqlite> select mutant_id, status, duration from mutant order by duration desc limit 1;
    mutant_id = cxx_add_to_sub:/tmp/sc-76UJhQXB4/fmt/include/fmt/format.h:684:23
       status = 3
     duration = 104

The mutant status is 3, which is a timeout according to the table above.

Deeper dive
***********

Exploration via SQLite is cool, but let's do some math and calculate the mutation
score using SQL.

To calculate mutation score, we will use the following formula:
``# of killed mutants / # of all mutants``, where killed means that the status
of a ``mutant`` is anything but ``Passed``.

Counting all the mutants is rather trivial but a bit lengthy, so let's create an SQL view:

.. code-block::

    sqlite> create view killed_mutants as select * from mutant where status <> 2;
    sqlite> select count(*) as killed from killed_mutants;
    killed = 4

With the number of killed mutants in place we can calculate the mutation score:

.. code-block::

    sqlite> select round(
        (select count(*) from killed_mutants) * 1.0 /
        (select count(*) from mutant) * 100) as score;
    score = 2.0

Gotchas
*******

One important thing to remember: by default Mull also stores ``stderr`` and ``stdout``
of each test run, which can blow up the size of the database by tens on gigabytes.

If you don't need the ``stdout/stderr``, then it is recommended to disable it via one of the following options ``--no-output``, ``--no-test-output``, ``--no-mutant-output``.

Alternatively, you can strip this information from the database using this query:

.. code-block::

    begin transaction;
    create temporary table t1_backup as select test_id, mutation_point_id, status, duration FROM execution_result;
    drop table execution_result;
    create table execution_result as select * FROM t1_backup;
    drop table t1_backup;
    commit;
    vacuum;
