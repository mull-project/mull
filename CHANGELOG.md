# Changelog

## [0.7.0] - TBD

 - TBD

## [0.6.0] - 04 Dec 2019

 - Switched to [libirm](https://github.com/mull-project/libirm) - generic library for LLVM mutations #566
 - Started building online documentation: https://mull.readthedocs.io
 - Introduce a more granular mutation operators: https://mull.readthedocs.io/en/latest/SupportedMutations.html
 - Fixed compilation database parsing issue #632 #629
 - Fixed gcc builds #630
 - Report warning when JIT cannot resolve undefined symbol #628
 - Added an option to whitelist mutants by paths (`-include-path=...`) #617
 - Added `-dry-run` options #616
 - Removed the old driver (`mull-driver`) #612
 - Added LLVM 9 support #609
 - Fixed a bug when even junk mutations were applied, resulting in a longer execution time #595
 - Sped up junk detection by an order of magnitude #582 
 - [All the changes](https://github.com/mull-project/mull/pulls?q=is%3Apr+merged%3A2019-08-29..2019-12-04)

## [0.5.0] - 28 Aug 2019

 - Print more info when original test fails #549
 - Reporting JSON to Mutation Testing Elements (initial support) #517, #507
 - Introduce `-exclude-path` option to filter mutations #553
 - Enable reporters configuration via `-reporters` option #555
 - Junk detection for the Scalar Value mutator #557
 - Junk detection for the Replace Assignment mutator #559
 - [All the changes](https://github.com/mull-project/mull/pulls?q=is%3Apr+merged%3A2019-08-11..2019-08-28)

## [0.4.0] - 11 Aug 2019

 - Fixed the original value use-after-free issue #532, #536
 - Automated release process on macOS #537
 - Fix incorrect handling of the NSW/NUW flags #540
 - Bring back abandoned sandbox approach #541, #542
 - Add LLVM 9 support #544
 - Include build date into the version info #546
 - Include Ubuntu 18.04 into release process #547
 - [All the changes](https://github.com/mull-project/mull/pulls?q=is%3Apr+merged%3A2019-06-03..2019-08-11)

## [0.3.0] - 02 Jun 2019

 - Add junk detection
   - AndOrReplacement #509
   - MathDiv #511
   - MathMul #511
   - ReplaceCall #512
 - Handle edge case for AND-OR mutator #501
 - Add better handling of edge cases when there is no debug information #519, #520, #524
 - Print more info when Junk Detector fails to parse a file #524
 - [All the changes](https://github.com/mull-project/mull/pulls?q=is%3Apr+merged%3A2019-05-03..2019-06-02)

## [0.2.0] - 02 May 2019

 - Add FreeBSD Package #503
 - Switch back to PackageMaker on macOS #504
 - Add internal target to validate mutants #500
 - Lots of refactoring
 - [All the changes](https://github.com/mull-project/mull/pulls?q=is%3Apr+merged%3A2019-04-03..2019-05-02)

## [0.1.0] - 02 Apr 2019

 - Initial Release

## [0.0.0] - 16 Apr 2016

 - Initial commit

