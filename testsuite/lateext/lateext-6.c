/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f -DCE_LATE_EXTERNALIZE -DCE_EXPORT_SYMBOLS=g -DCE_KEEP_INCLUDES" }*/

int g(void)
{
  return 0;
}

int f(void)
{
  return g();
}

/* { dg-final { scan-tree-dump-not "int g\(void\)" } } */
/* { dg-final { scan-tree-dump "static int \(\*klpe_g\)\(void\) __attribute__\(\(used\)\);" } } */
/* { dg-final { scan-tree-dump "return \(\*klpe_g\)\(\);" } } */
