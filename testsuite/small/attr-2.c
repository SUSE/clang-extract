/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f -DCE_NO_EXTERNALIZATION" }*/
int a __attribute__((unused));

int f(int x)
{
  return a = x;
}

/* { dg-final { scan-tree-dump "int a __attribute__\(\(unused\)\);" } } */
