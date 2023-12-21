/* { dg-compile "-fdump-ipa-clones -O3 -g3"} */
/* { dg-options "-where-is-inlined g -graphviz"} */


static inline int g(void)
{
  return 42;
}

int f()
{
  return g();
}

int main(void)
{
  return f();
}


/* { dg-final { scan-tree-dump "strict digraph {" } } */
/* { dg-final { scan-tree-dump "\"__float128\" -> \"float\"" } } */
/* { dg-final { scan-tree-dump "\"float\" -> \"main\""  } } */
