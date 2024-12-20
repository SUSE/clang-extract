/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f -DCE_EXPORT_SYMBOLS=g -DCE_RENAME_SYMBOLS" }*/
static int g()
{
  volatile int x = 3;
  return x;
}

int f(void)
{
  return g();
}

/* { dg-final { scan-tree-dump-not "static int g" } } */
/* { dg-final { scan-tree-dump "static int \(\*klpe_g\)\(\) __attribute__\(\(used\)\);|__attribute__\(\(used\)\) static int \(\*klpe_g\)\(\);" } } */
/* { dg-final { scan-tree-dump "int klpp_f\(void\)\n{\n *return \(\*klpe_g\)\(\)" } } */
