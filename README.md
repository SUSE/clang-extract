# clang-extract

A tool to extract code content from source files using the clang and LLVM infrastructure.

Given a C source file and the name of one or more functions, clang-extract
produces a new, self-contained source file with only what those functions need
to compile: the functions themselves and every type, macro, variable and
declaration they depend on.  Everything else is dropped.  It can also rewrite
the extracted code (_externalization_, symbol renaming) so it can be used to
build livepatches for the Linux kernel and userspace programs loaded with libpulp.

The project ships three tools:

- `clang-extract`: the code extractor itself.
- `ce-inline`: checks where functions got inlined, based on gcc `.ipa-clones`
  files, debuginfo and the kernel `Module.symvers`.
- `ce-includetree`: prints the tree of `#include`s of a source file and what
  clang-extract would do with each header.

## Getting started

### Compiling clang-extract

clang-extract requires clang, LLVM, libelf, zlib, meson and ninja in order to build.
On openSUSE, you can install them by running:
```
$ sudo zypper install clang23 clang23-devel libclang-cpp23 \
       clang-tools libLLVM23 llvm23 llvm23-devel libelf-devel meson ninja \
       zlib-devel libzstd-devel
```
LLVM 16 up to LLVM 23 are supported.  LLVM 18 and higher is advised, since
those versions are better tested; you might find issues with LLVM 16 and 17.
The `contrib/suse-install-llvm.sh <VERSION>` script installs the needed
packages for a given LLVM version on openSUSE Tumbleweed.

Once you have all those packages installed, you must setup the meson build system in order
to compile. You can run either `build-debug.sh` for a debug build with no optimization
and debug flags enabled for development, or a full optimized build with
`build-release.sh`.  Those scripts will create a `build` folder where you can `cd` into
and invoke `ninja` for it to build.  Example:
```
$ ./build-release.sh
$ cd build
$ ninja
```

Then the `clang-extract`, `ce-inline` and `ce-includetree` binaries will be
available for you in the `build` folder.

### Testing clang-extract

clang-extract has automated testing. Running the testsuite is as easy as running:
```
$ ninja test
```
inside the `build` directory.  Test results are written into `*.log` files in the
build folder.  The tests in the `testsuite` folder are also a good source of
examples of what clang-extract can do.

## Using clang-extract
Clang-extract currently only supports C projects. Assuming clang-extract is
compiled, it can be used to extract code content from projects using the
following steps.

1. Find, in the project, the function you want to extract, and which file it is in.
2. Compile the project and grab the command line passed to the compiler.
3. Replace `gcc` with `clang-extract`.
4. Pass `-DCE_NO_EXTERNALIZATION -DCE_EXTRACT_FUNCTIONS=function -DCE_OUTPUT_FILE=/tmp/output.c`  to clang-extract.
5. Done. In `/tmp/output.c` will have everything necessary for  `function` to compile without any external dependencies.

If you want to run clang-extract over a whole project at once instead of a
single file, see [Running on an entire project](#running-on-an-entire-project-cc-mode).

### Trivial example

Lets show how clang-extract works with a trivial example. Save the following code as a.c:
```c
#include <stdlib.h>
#include <stdio.h>

void *unused_function(void)
{
  return malloc(1024);
}

int main(int argc, char *argv[])
{
  puts("Hello, world!");
  return 0;
}
```
compiling this code with clang would be:
```
$ clang a.c -O2 -o a
```

Note that the source code of `a.c` contain unused functions. In this case, clang-extract can be
used to extract only the functions actually needed. In this case, extract the `main` function:
```
$ clang-extract a.c -O2 -o a -DCE_EXTRACT_FUNCTIONS=main -DCE_OUTPUT_FILE=out.c
```
on the output file `out.c`, you will see the following code:
```c
/** clang-extract: from /usr/include/stdio.h:718:1 */
extern int puts (const char *__s);

/** clang-extract: from a.c:9:1 */
int main(int argc, char *argv[])
{
  puts("Hello, world!");
  return 0;
}
```
Notice how *any reference to unused_function is removed* and *all headers has been removed* and replaced by a declaration of
`puts`. The output code can be compiled with the same flags used to compile the original code:
```
$ clang out.c -O2 -o a
```

Multiple functions can be extracted at once by separating them with commas,
for example `-DCE_EXTRACT_FUNCTIONS=main,foo,bar`.

### Keeping includes

By default every header is expanded, meaning that only the needed declarations
from them are copied into the output.  If you would rather keep the
`#include` directives, pass `-DCE_KEEP_INCLUDES`.  Using it on the trivial
example above gives:
```c
#include <stdlib.h>
#include <stdio.h>

/** clang-extract: from a.c:9:1 */
int main(int argc, char *argv[])
{
  puts("Hello, world!");
  return 0;
}
```

`-DCE_KEEP_INCLUDES=<policy>` selects which headers are kept and which are
expanded.  The following expansion policies are supported:
 - `nothing`: Do not expand any header.
 - `everything`: Expand all headers.  Has the same semantic effect of not passing `-DCE_KEEP_INCLUDES`, but forces clang-extract to pass it through its header expansion logics (slow and very likely buggy!).
 - `kernel`: Special policy used by the kernel livepatching developers.
 - `system`: Keep all system headers installed in `/usr/include`, etc.
 - `compiler`: Keep all compiler-specific headers, such as `stdatomic.h`. Useful if you want to expand everything but still want to ensure compatibility with other compilers.
 - `projecthack`: Keep all system headers, and also try to find the project-specific headers in the system folder.  May not be 100% reliable.

For example, consider a project with a private header `include/util.h`:
```c
#ifndef UTIL_H
#define UTIL_H

#include <string.h>

#define SQUARE(x) ((x) * (x))

struct point { int x, y; };

static inline int dist2(struct point a, struct point b)
{
  return SQUARE(a.x - b.x) + SQUARE(a.y - b.y);
}

int unrelated(int);
#endif
```
and `geo.c`:
```c
#include <stdio.h>
#include "util.h"

int closest(struct point *pts, int n, struct point p)
{
  int best = 0;
  for (int i = 1; i < n; i++)
    if (dist2(pts[i], p) < dist2(pts[best], p))
      best = i;
  return best;
}

void print_point(struct point p)
{
  printf("(%d, %d)\n", p.x, p.y);
}
```
Using the `system` policy keeps the system headers but expands the
project-private `util.h`, so the output does not depend on the project's
include folder anymore:
```
$ clang-extract geo.c -Iinclude -DCE_EXTRACT_FUNCTIONS=closest -DCE_OUTPUT_FILE=out.c -DCE_KEEP_INCLUDES=system
```
```c
#include <stdio.h>
#include <string.h>
/** clang-extract: from include/util.h:4:9 */
#define SQUARE(x) ((x) * (x))

/** clang-extract: from include/util.h:5:1 */
struct point { int x, y; };

/** clang-extract: from include/util.h:6:1 */
static inline int dist2(struct point a, struct point b)
{
  return SQUARE(a.x - b.x) + SQUARE(a.y - b.y);
}

/** clang-extract: from geo.c:4:1 */
int closest(struct point *pts, int n, struct point p)
{
  int best = 0;
  for (int i = 1; i < n; i++)
    if (dist2(pts[i], p) < dist2(pts[best], p))
      best = i;
  return best;
}
```
Notice that `unrelated` and `print_point` were not copied.  Policies can be
refined with `-DCE_EXPAND_INCLUDES=<headers>` and
`-DCE_NOT_EXPAND_INCLUDES=<headers>`.  For example,
`-DCE_KEEP_INCLUDES=nothing -DCE_EXPAND_INCLUDES=util.h` produces the same
result as above, while `-DCE_KEEP_INCLUDES=nothing` alone keeps
`#include "util.h"` in the output.

Use `-DCE_NO_DUPLICATED_INCLUDES` to have clang-extract treat duplicated
includes as superfluous and remove them.  You may also want to use
`clang-tidy` to cleanup the generated file afterwards:
```
$ clang-tidy -checks='-*,readability-duplicate-include,misc-include-cleaner' -fix <out.c>
```

#### Inspecting the include tree

`ce-includetree` takes the same command line as clang-extract and prints the
include tree of the file, together with what the selected policy decides for
each header:
```
$ ce-includetree geo.c -Iinclude -DCE_KEEP_INCLUDES=system
geo.c Expand: 1 Output: 0 NotExpand: 0 NotOutput: 0 -include: 0
  /usr/include/stdio.h Expand: 0 Output: 1 NotExpand: 0 NotOutput: 0 -include: 0
    /usr/include/bits/libc-header-start.h Expand: 0 Output: 0 NotExpand: 0 NotOutput: 0 -include: 0
    ...
```
This is useful to understand why a header was (or was not) expanded.

### Symbol Externalization

Code transformation is very often needed when generating livepatches. For example,
if we need to call functions that are not exported in the program (i.e. private),
we need to do a process called _externalization_.

Externalization works by redeclaring the original symbol as a pointer to its original
symbol. By doing that we avoid linking issues that may come from using an private
symbol.

Externalization is automatically enabled by default and can be disabled by providing the
`-DCE_NO_EXTERNALIZATION` option.

### Manual externalization

For example, with the following input:
```c
#include <stdio.h>

static int counter;

int function(void)
{
  return counter++;
}

int main(void)
{
  puts("Hello, world!");
  return function();
}
```
calling clang-extract with:
```
$ clang-extract b.c -DCE_EXTRACT_FUNCTIONS=main -DCE_OUTPUT_FILE=out.c -DCE_EXPORT_SYMBOLS=function
```
will externalize the function `function`, as the following output shows:
```c
/** clang-extract: from /usr/include/stdio.h:718:1 */
extern int puts (const char *__s);

/** clang-extract: from b.c:5:1 */
__attribute__((used)) static int (*klpe_function)(void);

/** clang-extract: from b.c:10:1 */
int main(void)
{
  puts("Hello, world!");
  return (*klpe_function)();
}
```
as one can see, the `function` was replaced by a pointer to a function `klpe_function`. On livepatching,
this pointer to function is filled with the address of the original function, bypassing any kind of
linking issues generated by symbol visibility.  Variables are externalized the
same way: every use of `counter` becomes `(*klpe_counter)`.

The opposite is also possible: `-DCE_NOT_EXPORT_SYMBOLS=<symbols>` forces the
listed symbols to never be externalized, even when the automatic analysis
(see below) decides that they should be.  Marking the same symbol in both
lists results in an error message.

#### Externalizing through macros

Rewriting every use of an externalized symbol makes the extracted function
look different from the original source, which makes reviewing a livepatch
harder.  With `-DCE_MACRO_EXTERNALIZATION` clang-extract keeps the function
bodies untouched and emits a macro that redirects the symbol to its pointer
instead:
```
$ clang-extract b.c -DCE_EXTRACT_FUNCTIONS=main,function -DCE_OUTPUT_FILE=out.c -DCE_EXPORT_SYMBOLS=counter -DCE_MACRO_EXTERNALIZATION
```
```c
/** clang-extract: from /usr/include/stdio.h:718:1 */
extern int puts (const char *__s);

/** clang-extract: from b.c:3:1 */
__attribute__((used)) static int *klpe_counter;

/** clang-extract: due to counter externalization */
#define counter	(*klpe_counter)

/** clang-extract: from b.c:5:1 */
int function(void)
{
  return counter++;
}

/** clang-extract: from b.c:10:1 */
int main(void)
{
  puts("Hello, world!");
  return function();
}
```

This also avoid other problems that may show up when externalizing symbols that
are declared in macros, like this example from openssl:
```c
extern const ASN1_ITEM *CMS_RecipientEncryptedKey_it(void);
#define ASN1_ITEM_rptr(ref) (ref##_it())
#define M_ASN1_new_of(type) (type *)ASN1_item_new(ASN1_ITEM_rptr(type))

void *foo(void)
{
    return (void *) M_ASN1_new_of(CMS_RecipientEncryptedKey);
}
```
because `foo` has an use of `CMS_RecipientEncryptedKey_it` that was generated
by macro expansions concatenating tokens, the externalizer will have trouble
renaming this variable, resulting in non-compilable code.  Using
`-DCE_MACRO_EXTERNALIZATION` in this case avoids this problem entirely.

#### Renaming extracted functions

A livepatch usually needs the new version of a function to have a different
name than the original one.  `-DCE_RENAME_SYMBOLS` renames every extracted
function with the `klpp_` prefix:
```
$ clang-extract b.c -DCE_EXTRACT_FUNCTIONS=function -DCE_OUTPUT_FILE=out.c -DCE_EXPORT_SYMBOLS=counter -DCE_RENAME_SYMBOLS
```
```c
/** clang-extract: from b.c:3:1 */
__attribute__((used)) static int *klpe_counter;

/** clang-extract: from b.c:5:1 */
int klpp_function(void)
{
  return (*klpe_counter)++;
}
```

When renaming, `-DCE_OUTPUT_FUNCTION_PROTOTYPE_HEADER=<file>` also writes a
header with the prototypes of the renamed functions, so other files can call
them:
```c
/** clang-extract: from b.c:5:1 */
int klpp_function(void);
```
This header is not self-compilable: it only contains the prototypes.

### Automatic externalization

clang-extract is able to automatically detect which symbols should be externalized if correct
information is given to it. For that, three switches are available for the user to provide
such information:

- `-DCE_DEBUGINFO_PATH=<path>`: Path to the debuginfo of the binary that will
  receive the livepatching. For compiled binaries with `-g`, this is embedded into the binary itself.
  With this clang-extract can discover which symbols are available and automatically mark the functions
  to be externalized.  For userspace livepatching, two comma-separated paths
  can be given: the stripped library (with `.dynsym`) and its separate
  `.debug` file.  For the kernel only the first one is used.
- `-DCE_IPACLONES_PATH=<path>`: Path containing a single `ipa-clones` or a folder with multiple `ipa-clones`
  file. This is used to verify the symbols that got inlined and may need to have its entire body copied to
  the output file.  These files are generated by gcc with `-fdump-ipa-clones`.
- `-DCE_SYMVERS_PATH=<path>`: Path containing the kernel `Modules.symvers` file, used by kernel livepatching
  to also externalize symbols that comes from modules that the livepatch do not want to depend upon.

The precision of the automatic analysis depends of the amount of information the user provides.  Clang-extract
will in any case try to do its best to figure out what is the best option when certain information is not
available.

#### Example

Save the following as `prog.c`:
```c
#include <stdio.h>

static int calls;

static int square(int x)
{
  calls++;
  return x * x;
}

int sum_of_squares(int a, int b)
{
  return square(a) + square(b);
}

int function(void)
{
  printf("%d (%d calls)\n", sum_of_squares(3, 4), calls);
  return 0;
}
```
and build it with gcc, asking it to dump the inlining decisions:
```
$ gcc -O2 -g -fdump-ipa-clones prog.c -shared -fPIC -o libprog.so
```
This generates `prog` and `prog.c.000i.ipa-clones`.  Now extract
`sum_of_squares` with that information:
```
$ clang-extract prog.c -O2 -g -fPIC -shared -DCE_EXTRACT_FUNCTIONS=sum_of_squares \
      -DCE_DEBUGINFO_PATH=libprog.so -DCE_IPACLONES_PATH=. -DCE_OUTPUT_FILE=out.c
```
```c
/** clang-extract: from /usr/include/stdio.h:370:1 */
extern int printf (const char *__restrict __format, ...);

/** clang-extract: from prog.c:3:1 */
__attribute__((used)) static int *klpe_calls;

/** clang-extract: from prog.c:5:1 */
static int square(int x)
{
  (*klpe_calls)++;
  return x * x;
}

/** clang-extract: from prog.c:11:1 */
int sum_of_squares(int a, int b)
{
  return square(a) + square(b);
}

/** clang-extract: from prog.c:16:1 */
int function(void)
{
  printf("%d (%d calls)\n", sum_of_squares(3, 4), (*klpe_calls));
  return 0;
}
```
A few things happened automatically:
- `calls` is a private variable of the original binary, so it was externalized.
- gcc inlined `square` into `sum_of_squares` and removed it, so there is no
  `square` symbol to externalize: its full body was copied instead.
- gcc inlined `sum_of_squares` into `function`, so patching `sum_of_squares` alone
  is not enough.  `function` was extracted as well.

Adding `-DCE_NOT_EXPORT_SYMBOLS=calls` to the command above would keep
`static int calls;` as a local copy instead of externalizing it.

#### Inspecting inlining decisions with ce-inline

`ce-inline` answers the same questions clang-extract asks itself, which is
useful to plan a livepatch.  Using the same `prog` from above, find where
`square` got inlined into:
```
$ ce-inline -ipa-files . -debuginfo libprog.so -where-is-inlined square
Mangled name    Demangled name  Type	Available?
function        function        FUNC	Private symbol
sum_of_squares  sum_of_squares  FUNC	Private symbol
```
and which functions got inlined into `sum_of_squares`:
```
$ ce-inline -ipa-files . -debuginfo prog -compute-closure sum_of_squares
Mangled name    Demangled name  Type	Available?
square          square          NOTYPE	Inlined
```
Pass `-csv` or `-graphviz` to get the output in CSV or graphviz `.dot`
format, and `-o <file>` to write it to a file.  For the kernel, also pass
`-symvers <path>`.

### Userspace livepatching with libpulp

clang-extract can generate the description (`.dsc`) file used by
[libpulp](https://github.com/SUSE/libpulp) to build userspace livepatches.
Continuing the example above:
```
$ clang-extract prog.c -O2 -g -DCE_EXTRACT_FUNCTIONS=sum_of_squares \
      -DCE_DEBUGINFO_PATH=libprog.so -DCE_IPACLONES_PATH=. -DCE_RENAME_SYMBOLS \
      -DCE_OUTPUT_FILE=livepatch.c -DCE_DSC_OUTPUT=livepatch.dsc
```
`livepatch.c` now contains `klpp_sum_of_squares` and `klpp_function`, and
`livepatch.dsc` describes which functions are replaced and which symbols must
be resolved for the externalized pointers:
```
LIVEPATH_CONTAINER
@libprog.so
sum_of_squares:klpp_sum_of_squares
function:klpp_function
#calls:klpe_calls
```
The first line, `LIVEPATH_CONTAINER`, is a placeholder for the path of the
compiled livepatch shared object, and the `@` line is the target library taken
from `-DCE_DEBUGINFO_PATH`.

### Linux kernel livepatching

When `-D__KERNEL__` is found in the command line, clang-extract enables the
logic needed for kernel livepatching: the `kernel` include policy is
available, `-DCE_SYMVERS_PATH` is used to decide what to externalize, and the
module name is taken from `-DKBUILD_MODNAME`.  The easiest way to get the
command line is to build the kernel with `make V=1` and copy the line that
compiles the file you are interested in.  If you use klp-build, the
`contrib/extract-ce-cmdline.sh` script extracts the clang-extract command line
from its log files, optionally running it under gdb with `--with-gdb`.

### Running on an entire project (CC mode)

Grabbing the command line of each file by hand does not scale when you do not
know in advance which file defines the function, or when the project has
thousands of files.  With `-DCE_CC=<compiler>` clang-extract behaves like an
ordinary C compiler: it first runs `<compiler>` with the original arguments,
so the build produces the object files, executables and libraries it expects,
and then extracts code from the file being compiled.  This way clang-extract
can be passed as `CC` to the project's build system.

In this mode:
- `-DCE_OUTPUT_BASEDIR=<dir>` is mandatory and must be an absolute path.  The
  output of each file is written under `<dir>`, mirroring the source tree
  (`src/foo.c` becomes `<dir>/src/foo.CE.c`).
- `-DCE_TRIGGER_ON_FULL_BODY=<functions>` restricts extraction to the files
  that contain the full body of all the listed functions.  Files that do not
  are compiled normally and silently skipped.
- Linking steps and other invocations that do not compile a C file are passed
  to the compiler untouched.
- If the compiler is called with `-fdump-ipa-clones`, the generated
  `.ipa-clones` file is loaded automatically, unless `-DCE_IPACLONES_PATH` is
  given.
- `-DCE_DSC_OUTPUT` without a value writes the `.dsc` file into
  `<dir>` as well.

For example, given a project built with a simple Makefile that has `closest`
from the previous examples defined somewhere in `src/`:
```
$ make CC="clang-extract -DCE_CC=gcc -DCE_OUTPUT_BASEDIR=$PWD/out \
                         -DCE_EXTRACT_FUNCTIONS=closest \
                         -DCE_TRIGGER_ON_FULL_BODY=closest \
                         -DCE_NO_EXTERNALIZATION"
$ find out -type f
out/src/geo.CE.c
```
The project builds as usual, and `out/src/geo.CE.c` contains `closest` with all
its dependencies.  The same idea applies to any build system that lets you
override the C compiler, and it has been used on projects as big as openSSL.

### Debugging clang-extract

- `-DCE_DUMP_PASSES` dumps the result of each transformation pass into files
  next to the input file, which is useful to find which pass is producing a
  wrong output.
- `-DCE_IGNORE_CLANG_ERRORS` makes clang-extract output code even if clang
  reported errors in the input.  Useful when the code is gcc-specific in a way
  clang does not accept.
- gcc-only flags that clang does not understand (e.g. `-mrecord-mcount`,
  `-fconserve-stack`) are silently ignored; `clang-extract --help` lists them.

##  Supported options

Clang-extract support many options which controls the output code:

- `-D__KERNEL__`                  Indicate that we are processing a Linux sourcefile, which triggers some special logics for kernel livepatching.
- `-DCE_EXTRACT_FUNCTIONS=<args>` Extract the functions specified in the <args> list, separated by commas.
- `-DCE_TRIGGER_ON_FULL_BODY=<args>` Only run the extraction if the full body of all symbols in <args> is found in the file.  Mostly useful with `-DCE_CC`.
- `-DCE_EXPORT_SYMBOLS=<args>`    Force externalization of symbols specified in the <args> list, separated by commas.
- `-DCE_NOT_EXPORT_SYMBOLS=<args>` Never externalize the symbols specified in the <args> list, separated by commas.
- `-DCE_OUTPUT_FILE=<arg>`        Output code to <arg> file.  Default is `<input>.CE.c`.
- `-DCE_OUTPUT_BASEDIR=<dir>`     Write the output under <dir>.  Mandatory, and must be an absolute path, when `-DCE_CC` is used.
- `-DCE_CC=<path>`                Run the C compiler in <path> to generate the ordinary build output, allowing clang-extract to be used as `CC` of a build system.
- `-DCE_NO_EXTERNALIZATION`       Disable symbol externalization.
- `-DCE_MACRO_EXTERNALIZATION`    Reference externalized symbols through a macro instead of rewriting each of their uses.
- `-DCE_LATE_EXTERNALIZE`         Enable late externalization (declare externalized variables later than the original).  May reduce code output when `-DCE_KEEP_INCLUDES` is enabled.
- `-DCE_RENAME_SYMBOLS`           Allow renaming of extracted symbols.
- `-DCE_OUTPUT_FUNCTION_PROTOTYPE_HEADER=<arg>` Output a header with the prototypes of the renamed functions.  This header is not self-compilable.
- `-DCE_DUMP_PASSES`              Dump the results of each transformation pass into files. Files will be dumped at the same path of the input files. Additional files are also generated on `/tmp/` folder.
- `-DCE_KEEP_INCLUDES`            Keep all possible `#include<file>` directives.
- `-DCE_KEEP_INCLUDES=<policy>`   Keep all possible `#include<file>` directives, but using the specified include expansion <policy>.  Valid values are `nothing`, `everything`, `kernel`, `system`, `compiler` and `projecthack`.
- `-DCE_EXPAND_INCLUDES=<args>`   Force expansion of the headers provided in <args>.
- `-DCE_NOT_EXPAND_INCLUDES=<args>` Force the following headers to **not** be expanded.
- `-DCE_NO_DUPLICATED_INCLUDES`   Treat all duplicated includes as superfluous so they can be removed.
- `-DCE_DEBUGINFO_PATH=<arg>`     Path to the compiled (ELF) object of the desired program to extract.  This is used to decide if externalization is necessary or not for given symbol.  Up to two comma-separated paths are accepted.
- `-DCE_IPACLONES_PATH=<arg>`     Path to gcc .ipa-clones files generated by gcc.  Used to decide if desired function to extract was inlined into other functions.
- `-DCE_SYMVERS_PATH=<arg>`       Path to kernel Modules.symvers file.  Only used when `-D__KERNEL__` is specified.
- `-DCE_DSC_OUTPUT=<arg>`         Libpulp .dsc file output, used for userspace livepatching.  With `-DCE_CC`, the path can be omitted.
- `-DCE_IGNORE_CLANG_ERRORS`      Ignore clang compilation errors in a hope that code is generated even if it won't compile.

For more switches, see
```
$ clang-extract --help
```
for more options.

## Supported features

Currently we only support projects written in C. Clang-extract is extensively tested with the Linux kernel, glibc and openSSL sourcecode.
C++ support is planned and clang-extract has some tests for it, but it can not handle libstdc++ headers yet.
