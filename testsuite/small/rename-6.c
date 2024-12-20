/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f,g, -DCE_EXPORT_SYMBOLS=a" }*/
static int a;

void f(int x)
{
  a = x;
}

int g()
{
  return a;
}

/* { dg-final { scan-tree-dump "static int \*klpe_a __attribute__\(\(used\)\);|__attribute__\(\(used\)\) static int \*klpe_a" } } */
/* { dg-final { scan-tree-dump "\(\*klpe_a\) = x;" } } */
/* { dg-final { scan-tree-dump "return \(\*klpe_a\);" } } */
