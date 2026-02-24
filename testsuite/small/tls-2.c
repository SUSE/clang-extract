/* { dg-options "-DCE_EXTRACT_FUNCTIONS=set_errno -DCE_EXPORT_SYMBOLS=__libc_errno" }*/
/* { dg-xfail }*/

extern __thread int __libc_errno __attribute__ ((tls_model ("initial-exec")));

void set_errno(int err)
{
  __libc_errno = err;
}

/* { dg-final { scan-tree-dump "__attribute__\(\(used\)\) static tls_index klpe___libc_errno_ti;" } } */
/* { dg-final { scan-tree-dump "__tls_get_addr\(klpe___libc_errno_ti\)" } } */
