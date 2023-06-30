/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f -DCE_NO_EXTERNALIZATION" }*/
enum AA {
  const0 = 0,
  const1 = 1,
};

int f() {
  return const1;
}

/* { dg-final { scan-tree-dump "const1 = 1" } } */
/* { dg-final { scan-tree-dump "return const1;" } } */
