/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f -DCE_NO_EXTERNALIZATION" }*/

struct A {
  int a;
};

template<typename T>
struct V {
  T v[30];
};

int f(V<A> *u)
{
  return u->v[0].a;
}

/* { dg-final { scan-tree-dump "struct A \{" } } */
/* { dg-final { scan-tree-dump "T v\[30\];" } } */
