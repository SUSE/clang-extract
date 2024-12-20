/* { dg-options "-DCE_EXTRACT_FUNCTIONS=g -DCE_EXPORT_SYMBOLS=f" }*/

int f();

int f()
{
  return 3;
}

int g()
{
  return f();
}

/* { dg-final { scan-tree-dump "static int \(\*klpe_f\)\(\) __attribute__\(\(used\)\);|__attribute__\(\(used\)\) static int \(\*klpe_f\)\(\)" } } */
/* { dg-final { scan-tree-dump "return \(\*klpe_f\)\(\);" } } */
