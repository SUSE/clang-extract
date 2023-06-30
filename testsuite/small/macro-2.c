/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f,g -DCE_NO_EXTERNALIZATION" }*/
#define A 0

int f(void) {
  #define C 3
  return C;
  #undef C
}

#define B 1

int g(void) {
  return B;
}

/* { dg-final { scan-tree-dump "#define C 3\n *return C;\n *#undef C" } } */
/* { dg-final { scan-tree-dump "#define B 1" } } */
/* { dg-final { scan-tree-dump "return B;" } } */
/* { dg-final { scan-tree-dump-not "#define A 0" } } */
