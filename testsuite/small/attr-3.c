/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f -DCE_NO_EXTERNALIZATION" }*/
int __attribute__((unused)) a;

int f(int x)
{
  return a = x;
}

/* { dg-final { scan-tree-dump "int __attribute__\(\(unused\)\) a;" } } */
