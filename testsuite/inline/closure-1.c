/* { dg-compile "-fdump-ipa-clones -O3 -g3"} */
/* { dg-options "-compute-closure main"} */

static inline int g(void)
{
  return 42;
}

static __attribute__((noinline)) h(void)
{
  return 43;
}

static int f()
{
  return g();
}

int main(void)
{
  return f() + h();
}


/* { dg-final { scan-tree-dump "g" } } */
/* { dg-final { scan-tree-dump "f" } } */
/* { dg-final { scan-tree-dump-not "h" } } */
