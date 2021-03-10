Incremental mutation testing
============================

Normally, Mull looks for mutations in all files of a project. Depending on a
project's size, a number of mutations can be very large, so running Mull
against all of them might be a rather slow process. Speed aside, an analysis of
a large mutation data sets can be very time consuming work to be done by a
user.

Incremental mutation testing is a feature that enables running Mull only on the
mutations found in Git Diff changesets. Instead of analysing all files and
functions, Mull only finds mutations in the source lines that are covered by
a particular Git Diff changeset.

Example: if a Git diff is created from a project's Git tree and the diff is only
one line, Mull will only find mutations in that line and will skip everything
else.

To enable incremental mutation testing, two arguments have to be provided to
Mull: ``-git-diff-ref=<branch or commit>`` and ``-git-project-root=<path>``
which is a path to a project's Git root path.

An additional debug option ``-debug`` can be useful for a visualization of how
exactly Mull whitelists or blacklists found source lines.

**Note:** Incremental mutation testing is an experimental feature. Things might
go wrong. If you encounter any issues, please report them on the
`mull/issues <https://github.com/mull-project/mull/issues>`_ tracker.
