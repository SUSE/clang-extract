/* { dg-compile "-fdump-ipa-clones -g3"} */
/* { dg-options "-csv"} */


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


/* { dg-final { scan-tree-dump "main;main;FUNC;" } } */
