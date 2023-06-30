/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f -DCE_NO_EXTERNALIZATION" }*/
enum {
  const1 = 1,
};

struct A {
  int v[const1];
};

int f(struct A *a) {
  return a->v[0];
}

/* { dg-final { scan-tree-dump "const1 = 1" } } */
/* { dg-final { scan-tree-dump "int v\[const1\];" } } */
/* { dg-final { scan-tree-dump "return a->v\[0\];" } } */
