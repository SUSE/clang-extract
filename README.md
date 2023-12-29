# clang-extract

A tool to extract code content from source files using the clang and LLVM infrastructure.

## Getting started

### Compiling clang-extract

clang-extract requires clang, LLVM, libelf, python3-pexpect, python3-psuti,
meson and ninja in order to build.
On OpenSUSE, you can install them by running:
```
$ sudo zypper install clang16 clang16-devel libclang-cpp16 \
       clang-tools libLLVM16 llvm16 llvm16-devel meson ninja \
       python311-psutil python311-pexpect python311-subprocess-tee python311-pytest
```
Older versions of clang and LLVM may also work, but is untested. We currently support
clang 16 or higher.

Once you have all those packages installed, you must setup the meson build system in order
to compile. You can run either `build-debug.sh` for a debug build with no optimization
and debug flags enabled for developtment, or a full optimized build with
`build-release.sh`.  Those scripts will create a `build` folder where you can `cd` into
and invoke `ninja` for it to build.  Example:
```
$ ./build-release.sh
$ cd build
$ ninja
```

Then the `clang-extract` binary will be available for you in the `build` folder.

### Testing clang-extract

clang-extract has automated testing. Running the testsuite is as easy as running:
```
$ ninja test
```
inside the `build` directory.  Test results are written into `*.log` files in the
build folder.

## Using clang-extract
Clang-extract currently only support C projects. Assuming clang-extract is compiled, it can be used to extract code content from projects using the following steps.

1. Find, in the project, the function you want to extract, and which file it is in.
2. Compile the project and grab the command line passed to the compiler.
3.  Replace `gcc` with `clang-extract`
4. Pass `-DCE_NO_EXTERNALIZATION -DCE_EXTRACT_FUNCTIONS=function -DCE_OUTPUT_FILE=/tmp/output.c`  to clang-extract.
5. Done. In `/tmp/output.c` will have everything necessary for  `function` to compile without any external dependencies.

### Example
Let's extract the function `__libc_malloc` from the glibc project. The steps are:
1. Compile the glibc project until `malloc.c` is compiled: `make -j8 | grep malloc.c`
2. Grab the command line:
```
gcc malloc.c -c -std=gnu11 -fgnu89-inline  -g -O2 -Wall -Wwrite-strings -Wundef -Werror -fmerge-all-constants -frounding-math -fno-stack-protector -fno-common -Wp,-U_FORTIFY_SOURCE -Wstrict-prototypes -Wold-style-definition -fmath-errno    -fPIE   -DMORECORE_CLEARS=2  -ftls-model=initial-exec     -I../include -I/home/giulianob/projects/glibc/build_glibc/malloc  -I/home/giulianob/projects/glibc/build_glibc  -I../sysdeps/unix/sysv/linux/x86_64/64  -I../sysdeps/unix/sysv/linux/x86_64  -I../sysdeps/unix/sysv/linux/x86/include -I../sysdeps/unix/sysv/linux/x86  -I../sysdeps/x86/nptl  -I../sysdeps/unix/sysv/linux/wordsize-64  -I../sysdeps/x86_64/nptl  -I../sysdeps/unix/sysv/linux/include -I../sysdeps/unix/sysv/linux  -I../sysdeps/nptl  -I../sysdeps/pthread  -I../sysdeps/gnu  -I../sysdeps/unix/inet  -I../sysdeps/unix/sysv  -I../sysdeps/unix/x86_64  -I../sysdeps/unix  -I../sysdeps/posix  -I../sysdeps/x86_64/64  -I../sysdeps/x86_64/fpu/multiarch  -I../sysdeps/x86_64/fpu  -I../sysdeps/x86/fpu  -I../sysdeps/x86_64/multiarch  -I../sysdeps/x86_64  -I../sysdeps/x86/include -I../sysdeps/x86  -I../sysdeps/ieee754/float128  -I../sysdeps/ieee754/ldbl-96/include -I../sysdeps/ieee754/ldbl-96  -I../sysdeps/ieee754/dbl-64  -I../sysdeps/ieee754/flt-32  -I../sysdeps/wordsize-64  -I../sysdeps/ieee754  -I../sysdeps/generic  -I.. -I../libio -I.  -D_LIBC_REENTRANT -include /home/giulianob/projects/glibc/build_glibc/libc-modules.h -DMODULE_NAME=libc -include ../include/libc-symbols.h  -DPIC  -DUSE_TCACHE=1   -DTOP_NAMESPACE=
glibc -o /home/giulianob/projects/glibc/build_glibc/malloc/malloc.o -MD -MP -MF /home/giulianob/projects/glibc/build_glibc/malloc/malloc.o.dt -MT /home/giulianob/projects/glibc/build_glibc/malloc/malloc.o
```
3. Replace `gcc` with `clang-extract` and add the extra parameters (removed `-Werror` since clang treats some things as errors where gcc doesn't:
```
clang-extract malloc.c -c -std=gnu11 -fgnu89-inline  -g -O2 -Wall -Wwrite-strings -fmerge-all-constants -frounding-math -fno-stack-protector -fno-common -Wp,-U_FORTIFY_SOURCE -Wstrict-prototypes -Wold-style-definition -fmath-errno    -fPIE   -DMORECORE_CLEARS=2  -ftls-model=initial-exec     -I../include -I/home/giulianob/projects/glibc/build_glibc/malloc  -I/home/giulianob/projects/glibc/build_glibc  -I../sysdeps/unix/sysv/linux/x86_64/64  -I../sysdeps/unix/sysv/linux/x86_64  -I../sysdeps/unix/sysv/linux/x86/include -I../sysdeps/unix/sysv/linux/x86  -I../sysdeps/x86/nptl  -I../sysdeps/unix/sysv/linux/wordsize-64  -I../sysdeps/x86_64/nptl  -I../sysdeps/unix/sysv/linux/include -I../sysdeps/unix/sysv/linux  -I../sysdeps/nptl  -I../sysdeps/pthread  -I../sysdeps/gnu  -I../sysdeps/unix/inet  -I../sysdeps/unix/sysv  -I../sysdeps/unix/x86_64  -I../sysdeps/unix  -I../sysdeps/posix  -I../sysdeps/x86_64/64  -I../sysdeps/x86_64/fpu/multiarch  -I../sysdeps/x86_64/fpu  -I../sysdeps/x86/fpu  -I../sysdeps/x86_64/multiarch  -I../sysdeps/x86_64  -I../sysdeps/x86/include -I../sysdeps/x86  -I../sysdeps/ieee754/float128  -I../sysdeps/ieee754/ldbl-96/include -I../sysdeps/ieee754/ldbl-96  -I../sysdeps/ieee754/dbl-64  -I../sysdeps/ieee754/flt-32  -I../sysdeps/wordsize-64  -I../sysdeps/ieee754  -I../sysdeps/generic  -I.. -I../libio -I.  -D_LIBC_REENTRANT -include /home/giulianob/projects/glibc/build_glibc/libc-modules.h -DMODULE_NAME=libc -include ../include/libc-symbols.h  -DPIC  -DUSE_TCACHE=1   -DTOP_NAMESPACE=glibc -o /home/giulianob/projects/glibc/build_glibc/malloc/malloc.o -MD -MP -MF /home/giulianob/projects/glibc/build_glibc/malloc/malloc.o.dt -MT /home/giulianob/projects/glibc/build_glibc/malloc/malloc.o -DCE_NO_EXTERNALIZATION -DCE_OUTPUT_FILE=/tmp/out.c -DCE_EXTRACT_FUNCTIONS=__libc_malloc
```
4. The output should be in `/tmp/out.c` and should be self-compilable. Check it by calling `$ gcc -c /tmp/out.c`. Here is the output for malloc: https://godbolt.org/z/6vrrTPoP9
##  Supported options

See
```
$ clang-extract --help
```
for more options.

## Project status
- Support for C projects: DONE.
- Support for OpenSSL and Glibc: DONE.
- Automatic testing: DONE.
- Symbol Externalization engine: DONE.
- Support for the Linux Kernel: DONE.
- Support for C++ code: On going.
- Support binary analysis for symbol externalization: DONE.
- Support for Macro Rewriting: Partial.
