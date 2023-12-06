/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f -DCE_NO_EXTERNALIZATION" }*/

namespace A
{
  namespace B {
    template <typename T>
    struct BB {
      T x;
    };
  }

  typedef B::BB<int> BBB;
};

int f(A::BBB b)
{
  return b.x;
}

/* { dg-final { scan-tree-dump "namespace A" } } */
/* { dg-final { scan-tree-dump "namespace B" } } */
/* { dg-final { scan-tree-dump " *struct BB" } } */
/* { dg-final { scan-tree-dump " *typedef B::BB<int> BBB" } } */
/* { dg-final { scan-tree-dump "return b.x;" } } */
