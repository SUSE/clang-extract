/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f -DCE_NO_EXTERNALIZATION" }*/
#define a a

int f()
{
  int a = 0;
  return a;
}

/* { dg-final { scan-tree-dump "#define a a" } } */
