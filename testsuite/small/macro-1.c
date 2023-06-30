/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f -DCE_NO_EXTERNALIZATION" }*/
#define A 0

int f(void) {
  return A;
}

#define B 1

int g(void) {
  return B;
}


/* { dg-final { scan-tree-dump "#define A 0" } } */
/* { dg-final { scan-tree-dump "return A;" } } */
/* { dg-final { scan-tree-dump-not "#define B 1" } } */
/* { dg-final { scan-tree-dump-not "return B;" } } */
