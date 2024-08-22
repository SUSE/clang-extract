/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f -DCE_EXPORT_SYMBOLS=g" }*/
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
/* { dg-final { scan-tree-dump "static int \(\*klpe_g\)\(\) __attribute__\(\(used\)\);" } } */
/* { dg-final { scan-tree-dump "int f\(void\)\n{\n *return \(\*klpe_g\)\(\)" } } */
