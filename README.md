# clang-extract

A tool to extract code content from source files using the clang and LLVM infrastructure.

## Getting started

### Compiling clang-extract

clang-extract requires clang, LLVM and libtooling. On OpenSUSE, you can install them by running:
```
$ sudo zypper install clang16 clang16-devel libclang-cpp16 \
       clang-tools libLLVM16 llvm16 llvm16-devel 
```
Older versions of clang and LLVM may also work, but is untested.
Once you have all those packages installed, compiling clang-extract is as easy as running
```
$ make
```

### Testing clang-extract

clang-extract has automated testing. You need a few python3 libraries installed for it to run:
```
$ sudo zypper install python311-subprocess-tee python311-pytest python311-pexpect python311-psutil
```
Then running the testsuite is as easy as running:
```
$ make check -j<num_jobs>
```
## Using clang-extract
Clang-extract currently only support C project, such as `glibc` and `openssl`. Assuming clang-extract is compiled, it can be used to extract code content from projects using the following steps.

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
glibc -o /home/giulianob/projects/glibc/build_glibc/malloc/malloc.o -MD -MP -MF /home/giulianob/projects/glibc/build_glibc/malloc/malloc.o.dt -MT /home/giulianob/projects/glibc/build_glibc/m
alloc/malloc.o
```
3. Replace `gcc` with `clang-extract` and add the extra parameters (removed `-Werror` since clang treats some things as errors where gcc doesn't:
```
~/projects/clang-extract/clang-extract malloc.c -c -std=gnu11 -fgnu89-inline  -g -O2 -Wall -Wwrite-strings -fmerge-all-constants -frounding-math -fno-stack-protector -fno-common -Wp,-U_FORTIFY_SOURCE -Wstrict-prototypes -Wold-style-definition -fmath-errno    -fPIE   -DMORECORE_CLEARS=2  -ftls-model=initial-exec     -I../include -I/home/giulianob/projects/glibc/build_glibc/malloc  -I/home/giulianob/projects/glibc/build_glibc  -I../sysdeps/unix/sysv/linux/x86_64/64  -I../sysdeps/unix/sysv/linux/x86_64  -I../sysdeps/unix/sysv/linux/x86/include -I../sysdeps/unix/sysv/linux/x86  -I../sysdeps/x86/nptl  -I../sysdeps/unix/sysv/linux/wordsize-64  -I../sysdeps/x86_64/nptl  -I../sysdeps/unix/sysv/linux/include -I../sysdeps/unix/sysv/linux  -I../sysdeps/nptl  -I../sysdeps/pthread  -I../sysdeps/gnu  -I../sysdeps/unix/inet  -I../sysdeps/unix/sysv  -I../sysdeps/unix/x86_64  -I../sysdeps/unix  -I../sysdeps/posix  -I../sysdeps/x86_64/64  -I../sysdeps/x86_64/fpu/multiarch  -I../sysdeps/x86_64/fpu  -I../sysdeps/x86/fpu  -I../sysdeps/x86_64/multiarch  -I../sysdeps/x86_64  -I../sysdeps/x86/include -I../sysdeps/x86  -I../sysdeps/ieee754/float128  -I../sysdeps/ieee754/ldbl-96/include -I../sysdeps/ieee754/ldbl-96  -I../sysdeps/ieee754/dbl-64  -I../sysdeps/ieee754/flt-32  -I../sysdeps/wordsize-64  -I../sysdeps/ieee754  -I../sysdeps/generic  -I.. -I../libio -I.  -D_LIBC_REENTRANT -include /home/giulianob/projects/glibc/build_glibc/libc-modules.h -DMODULE_NAME=libc -include ../include/libc-symbols.h  -DPIC  -DUSE_TCACHE=1   -DTOP_NAMESPACE=glibc -o /home/giulianob/projects/glibc/build_glibc/malloc/malloc.o -MD -MP -MF /home/giulianob/projects/glibc/build_glibc/malloc/malloc.o.dt -MT /home/giulianob/projects/glibc/build_glibc/malloc/malloc.o -DCE_NO_EXTERNALIZATION -DCE_OUTPUT_FILE=/tmp/out.c -DCE_EXTRACT_FUNCTIONS=__libc_malloc
```
4. The output should be in `/tmp/out.c` and should be self-compilable. Check it by calling `$ gcc -c /tmp/out.c`. Here is the output for malloc: https://godbolt.org/z/6vrrTPoP9
##  Supported options

You can pass the following options to clang-extract:
- `-DCE_OUTPUT_FILE=<out>` File to write to. Else the output will be deduced from the input file.
-  `-DCE_EXTRACT_FUNCTIONS=f,g,h,...` The functions that should be extracted.
- `-DCE_DISABLE_EXTERNALIZATION` Disable symbol externalization. Symbol externalization would rewrite references to private symbols as pointers to addresses, which would be initialized by `libpulp`. This currently do not work as intended as clang-extract has no engine to analyze where the symbol is in the original library, so always pass this option at the moment.

## Project status
- Support for C projects: DONE.
- Support for OpenSSL and Glibc: DONE.
- Automatic testing: Done.
- Symbol Externalization engine: Done.
- Support for the Linux Kernel: TBI.
- Support for C++ code: TBI.
- Support binary analysis for symbol externalization: TBI.
- Support for Macro Rewriting: Partial.
