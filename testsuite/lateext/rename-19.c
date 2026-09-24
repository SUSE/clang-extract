/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f -DCE_EXPORT_SYMBOLS=h,g -DCE_MACRO_EXTERNALIZATION -DCE_LATE_EXTERNALIZE" }*/

int g(int);

int h(int x)
{
  return g(x);
}

int g(int x)
{
  return h(x);
}

int f(void)
{
  return g(3);
}

/* { dg-final { scan-tree-dump "return g\(3\);" } } */
/* { dg-final { scan-tree-dump "static int \(\*klpe_g\)\(int\) __attribute__\(\(used\)\);|__attribute__\(\(used\)\) static int \(\*klpe_g\)\(int\);" } } */
