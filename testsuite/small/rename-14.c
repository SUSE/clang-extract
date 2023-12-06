/* { dg-options "-DCE_EXTRACT_FUNCTIONS=g -DCE_RENAME_SYMBOLS" }*/
int f(int x)
{
  return x;
}

struct AAA
{
  int a;
};

static void *h(int);

struct AAA* g(int x)
{
  static struct AAA aa;
  aa.a = x;
  f(x);
  h(x);
  return &aa;
}

static void *h(int x)
{
  return (void *)g(x);
}

/* { dg-final { scan-tree-dump "struct AAA\* klpp_g\(int x\)" } } */
/* { dg-final { scan-tree-dump "return \(void \*\)klpp_g\(x\);" } } */
/* { dg-final { scan-tree-dump "f\(x\);" } } */
