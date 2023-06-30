/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f -DCE_NO_EXTERNALIZATION" }*/
enum AA {
  const0 = 0,
  const1 = 1,
};

int var[const1];

int f()
{
  return var[0];
}

/* { dg-final { scan-tree-dump "const1 = 1" } } */
/* { dg-final { scan-tree-dump "int var\[const1\]" } } */
/* { dg-final { scan-tree-dump "return var\[0\]" } } */
