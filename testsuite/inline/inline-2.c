/* { dg-compile "-fdump-ipa-clones -g3"} */
static inline int g(void)
{
  return 42;
}

__attribute__((noinline)) int f()
{
  return g();
}

int main(void)
{
  return f();
}


/* { dg-final { scan-tree-dump "_DYNAMIC" } } */
/* { dg-final { scan-tree-dump "f" } } */
/* { dg-final { scan-tree-dump-not "crtstuff.c" } } */
