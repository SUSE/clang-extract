/* { dg-compile "-fdump-ipa-clones -O3 -g3"} */
/* { dg-options "-where-is-inlined g"} */

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


/* { dg-final { scan-tree-dump "f" } } */
/* { dg-final { scan-tree-dump "g" } } */
/* { dg-final { scan-tree-dump "FUNC\tPublic" } } */
