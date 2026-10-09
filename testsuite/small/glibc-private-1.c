/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f -DCE_EXPORT_SYMBOLS=__ctype_init,puts -DCE_DEBUGINFO_PATH=$libc_so" }*/

/* Symbols with GLIBC_PRIVATE version can't be referenced directly from
   outside of glibc, hence they must always be externalized even though they
   are in the .dynsym.  Public symbols (e.g. puts) must not be.  */

void __ctype_init(void);
int puts(const char *);

int f(void)
{
  __ctype_init();
  return puts("hello");
}

/* { dg-final { scan-tree-dump "static void \(\*klpe___ctype_init\)\(void\) __attribute__\(\(used\)\);|__attribute__\(\(used\)\) static void \(\*klpe___ctype_init\)\(void\);" } } */
/* { dg-final { scan-tree-dump "\(\*klpe___ctype_init\)\(\);" } } */
/* { dg-final { scan-tree-dump "int puts\(const char \*\);" } } */
/* { dg-final { scan-tree-dump "return puts\(\"hello\"\);" } } */
/* { dg-final { scan-tree-dump-not "klpe_puts" } } */
