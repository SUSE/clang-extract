/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f -DCE_NO_EXTERNALIZATION" }*/
typedef enum {
  const1 = 1,
  const2 = 2,
} AA;

struct A {
  int v[const1];
};

int f(struct A *a) {
  return a->v[0];
}

/* { dg-final { scan-tree-dump "const1 = 1" } } */
/* { dg-final { scan-tree-dump "int v\[const1\];" } } */
/* { dg-final { scan-tree-dump "return a->v\[0\];" } } */
