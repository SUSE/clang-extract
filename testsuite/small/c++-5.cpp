/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f -DCE_NO_EXTERNALIZATION" }*/

struct A {
  int a;
};

template<typename T>
struct V {
  T v[30];
};

typedef V<A> VA;

int f(VA *u)
{
  return u->v[0].a;
}

/* { dg-final { scan-tree-dump "struct A \{" } } */
/* { dg-final { scan-tree-dump "T v\[30\];" } } */
/* { dg-final { scan-tree-dump "typedef" } } */
