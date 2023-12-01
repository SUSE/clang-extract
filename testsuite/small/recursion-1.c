/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f -DCE_NO_EXTERNALIZATION" }*/

int f(void)
{
  int global = global;
  return global;
}

/* { dg-final { scan-tree-dump "int global = global;" } } */
/* { dg-final { scan-tree-dump "return global;" } } */
