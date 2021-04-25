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

Typical use cases
-----------------

Under the hood, Mull runs ``git diff`` from a project's root folder. There
are at least three reasonable options for using the ``-git-diff-ref`` argument:

1. ``-git-diff-ref=origin/main``

   Mull is run from a branch with a few commits against a main branch such as
   ``main``, ``master`` or equivalent. This is what you get from your branch when you
   simply do ``git diff origin/master``. This way you can also test your branch
   if you have Mull running as part of your CI workflow.

2. ``-git-diff-ref=.`` (unstaged), ``-git-diff-ref=HEAD`` (unstaged + staged)

   Mull is run against a diff between the "unclean" tree state and your last
   commit. This use case is useful when you want to check your work-in-progress
   code with Mull before committing your changes.

3. ``-git-diff-ref=COMMIT^!``

   Mull is run against a diff of a specific commit (see also
   `How can I see the changes in a Git commit? <https://stackoverflow.com/questions/17563726/how-can-i-see-the-changes-in-a-git-commit>`_
   ). This option should be used with caution because Mull does not perform
   a ``git checkout`` to switch to a given commit's state. Mull always stands
   on top of the existing tree, so if a provided commit has already been
   overridden by more recent commits, Mull will not produce the results for
   that earlier commit which can result in a misleading information in the
   mutation reports. Use this option only if you are sure that no newer commits
   in your Git tree have touched the file(s) you are interested in.
