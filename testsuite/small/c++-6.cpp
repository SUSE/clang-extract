/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f -DCE_NO_EXTERNALIZATION" }*/

struct A {
  int a;
};

template<typename T = A>
struct V {
  T v[30];
};

int f(void)
{
  V u;
  return u.v[0].a;
}

/* { dg-final { scan-tree-dump "struct A \{" } } */
/* { dg-final { scan-tree-dump "T v\[30\];" } } */
/* { dg-final { scan-tree-dump "V u" } } */
