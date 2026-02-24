/* { dg-options "-DCE_EXTRACT_FUNCTIONS=set_errno -DCE_EXPORT_SYMBOLS=__libc_errno" }*/

extern __thread int __libc_errno __attribute__ ((tls_model ("initial-exec")));

void set_errno(int err)
{
  __libc_errno = err;
}

/* { dg-final { scan-tree-dump "__attribute__\(\(used\)\) static __thread int \*klpe___libc_errno;|static __thread int \*klpe___libc_errno __attribute__\(\(used\)\);" } } */
/* { dg-final { scan-tree-dump "\(\*klpe___libc_errno\) = err" } } */
