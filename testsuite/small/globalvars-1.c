/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f -DCE_NO_EXTERNALIZATION" }*/

int global = 1;

int f(void)
{
  return global;
}

/* { dg-final { scan-tree-dump "int global = 1;" } } */
/* { dg-final { scan-tree-dump "return global;" } } */
