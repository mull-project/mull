# Configuration

Mull uses [yaml](http://yaml.org) format to control its behavior.

Below you can find configuration options and their explanation along with
the real-world examples at the end of the document.

### Configuration options

---
```
project_name: string
```
Project name parameter is used by `SQLiteReporter` to compose a name of an output SQLite file.

---
```
test_framework: string
```
Tells Mull which test framework adapter to use.

Supported test frameworks:

 - `GoogleTest`: default value. Works out of the box except for typed tests (see #15).
 - `CustomTest`: used for a project with a custom test framework. See `custom_tests` below.
 - `SimpleTest`: used for testing only.

---
```
bitcode_file_list: path (string)
```
This is the most important part of the configuration. It points to a plain text
file that contains newline separated list of bitcode files which Mull has to process.

---
```
dynamic_library_file_list: path (string)
```

Similar to the `bitcode_file_list`, but the file contains new-line separated
list of dynamic libraries(`*.dylib`, `*.so`) required for execution of a program under test.

---
```
object_file_list: path (string)
```

Similar to the `bitcode_file_list`, but the file contains a newline separated
list of object files(`*.o`) required for execution of a program under test.

---
```
mutators: an array of strings
```

Mutators can be specified either by their names or by groups.

 - `math`: group, includes 4 next mutators
 - `math_add_mutator`: replaces `+` with `-`
 - `math_sub_mutator`: replaces `-` with `+`
 - `math_div_mutator`: replaces `/` with `*`
 - `math_mul_mutator`: replaces `*` with `/`
 - `conditional`: group, includes 3 next mutators
 - `and_to_or_replacement_mutator`: replaces `&&` with `||`
 - `or_to_and_replacement_mutator`: replaces `||` with `&&`
 - `negate_mutator`: negates all conditions, e.g.: `!=` -> `=`, `<` -> `>=`
 - `conditionals_boundary_mutator`: mutates boundaries, e.g.: `<` -> `<=`, `<=` -> `<`
 - `functions`: group, includes 2 next mutators
 - `replace_call_mutator`: replaces a call to a function returning integer with `42`
 - `remove_void_function_mutator`: removes a call to a function returning `void`
 - `constant`: group, includes only 1 next mutator
 - `scalar_value_mutator`: replaces the constants with other values: zero -> 1,
   non-zero -> zero
 - `default`: group, includes `math_add_mutator`, `negate_mutator`, and
   `remove_void_function_mutator`
 - `experimental`: group, includes all mutators except of `default` ones
 - `all`: group, includes all mutators

---
```
tests: an array of strings
```

Lists names of tests that should be taken into account. All the tests that are not in this list are ignored.

---
```
exclude_locations: an array of strings
```
Skips mutation based on its on-disk location.

---
```
dry_run: boolean
```

Possible values: `true`/`enabled`, `false`/`disabled`. Defaults to `false`.

When enabled Mull finds mutations, but does not execute them.

---
```
fail_fast: boolean
```
Possible values: `true`/`enabled`, `false`/`disabled`. Defaults to `false`.

A mutant may be accessible from many tests. Normally, Mull executes a mutant
against each test separately. When `fail_fast` option is enabled, Mull stops
running tests against a mutant as soon as on the tests fail.

---
```
use_cache: boolean
```
Possible values: `true`/`yes`, `false`/`no`. Defaults to `false`.

Saves compiled object files on disk to reuse on next runs.

---
```
cache_directory: path (string)
```
A path to a directory where Mull stores cached object files. Defaults to `/tmp/mull_cache`.
The directory is created if it does not exist. If a directory cannot be
created, then Mull disables on-disk caching.

---
```
timeout: milliseconds (integer)
```
Tells Mull to terminate test execution after the given timeout.

---
```
max_distance: integer
```
If a function `A` calls function `B`, and the function `B` calls function `C`,
Then the distance between `A` and `C` is two. The `max_distance` option tells
Mull to ignore all mutants that are too far away from a test function. Defaults
to `128`.

---
```
junk_detection:
  detector: name (string)
  enabled: boolean
  cxx_compdb_dir: path (string)
  cxx_compilation_flags: string
```

Some mutations found at the Bitcode level cannot be reproduced by a programmer
at the source code level. Mull comes with `CXXJunkDetector` that filters out
some junk mutations.

`CXXJunkDetector` is based on `clangTooling`, to parse source files
correctly `clangTooling` needs either compilation database, or set of flags used
for a compilation.

By default junk detection is disabled. See examples below.

---
```
parallelization:
  workers: integer
  test_execution_workers: integer
  mutant_execution_workers: integer
```

Mull can run most of the tasks in parallel. It does so by default.
Tests can be run sequentially if they are not designed to be run in parallel.

When running tests, Mull reuses one JIT stack across several threads. Though,
when executing mutants, it creates a JIT stack per-thread, which may consume a significant amount of RAM. In this case, it may make sense to use fewer threads
for mutant execution to prevent slowdown because of memory swapping.

By default Mull uses [`std::thread::hardware_concurrency()`](https://en.cppreference.com/w/cpp/thread/thread/hardware_concurrency) 
number of threads.

---
```
custom_tests:
  - name: string
    method: method name
    program: program name
    arguments: string
```

This option is used with `CustomTest`. If a project uses a custom test suite,
then one can provide a list of tests manually. Each test entry should have
a unique name, a name of a program (`argv[0]`), arguments (`argv`) required to
run the test program, and the name of a test method. The method name should
contain a mangled name of a test function.

If `CustomTest` used and no `custom_tests` specified, then Mull picks the whole program as a single test, i.e., the custom test definition looks like the
following:

```
custom_tests:
  - name: custom-test
    method: main
    program: mull
    arguments: ""
```

### Examples

##### LLVM ADT test suite:

```
project_name: LLVM_ADTTests
bitcode_file_list: /opt/examples/llvm_build/adttests.bitcode.list
use_cache: yes
cache_directory: /opt/examples/llvm_build/mull-cache
fail_fast: enabled

mutators:
  - default

junk_detection:
  detector: cxx
  enabled: yes
  cxx_compdb_dir: /opt/examples/llvm_build/
```

##### OpenSSL bio enc test suite

```
project_name: OpenSSL_bio_enc_test
test_framework: CustomTest
bitcode_file_list: /opt/examples/openssl/bio_enc_test.bitcode.list
fork: enabled
dry_run: disabled
fail_fast: disabled
use_cache: yes
cache_directory: /opt/examples/openssl/bio_enc_test_cache
max_distance: 5
timeout: 10000
exclude_locations:
  - testutil
mutators:
  - math_add_mutator

tests:
  - test_bio_enc_aes_128_cbc

custom_tests:
  - name: test_bio_enc_aes_128_cbc
    method: test_bio_enc_aes_128_cbc
    program: bio_enc_test
    arguments: [ test_bio_enc_aes_128_cbc ]

  - name: test_bio_enc_aes_128_ctr
    method: test_bio_enc_aes_128_ctr
    program: bio_enc_test
    arguments: [ test_bio_enc_aes_128_ctr ]

  - name: test_bio_enc_aes_256_cfb
    method: test_bio_enc_aes_256_cfb
    program: test_char
    arguments: [ test_bio_enc_aes_256_cfb ]

  - name: test_bio_enc_aes_256_ofb
    method: test_bio_enc_aes_256_ofb
    program: bio_enc_test
    arguments: [ test_bio_enc_aes_256_ofb ]

  - name: test_bio_enc_chacha20
    method: test_bio_enc_chacha20
    program: bio_enc_test
    arguments: [ test_bio_enc_chacha20 ]

  - name: test_bio_enc_chacha20_poly1305
    method: test_bio_enc_chacha20_poly1305
    program: bio_enc_test
    arguments: [ test_bio_enc_chacha20_poly1305 ]

junk_detection:
  detector: cxx
  enabled: yes
  cxx_compilation_flags: -DDSO_DLFCN -DHAVE_DLFCN_H -DNDEBUG -DOPENSSL_THREADS -DOPENSSL_NO_STATIC_ENGINE -DOPENSSL_PIC -DOPENSSLDIR="/usr/local/ssl" -DENGINESDIR="/usr/local/lib/engines-1.1" -D_REENTRANT -arch x86_64 -DL_ENDIAN -Wall  -flto -g -O0 -I include -I /opt/examples/openssl -I /opt/examples/openssl/crypto/include -I /opt/examples/openssl/crypto/modes
```

