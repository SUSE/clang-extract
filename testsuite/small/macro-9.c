/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f -DCE_NO_EXTERNALIZATION" }*/
enum A {
  const1 = 1,
};

#define MACRO const1

struct AA {
  int v[MACRO];
};

int f(struct AA *a)
{
  return a->v[0];
}

/* { dg-final { scan-tree-dump "const1 = 1" } } */
/* { dg-final { scan-tree-dump "#define MACRO const1" } } */
/* { dg-final { scan-tree-dump "int v\[MACRO\];" } } */
