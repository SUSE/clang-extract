/* { dg-options "-DCE_EXTRACT_FUNCTIONS=g -DCE_RENAME_SYMBOLS" }*/
int f(int x)
{
  return x;
}

struct AAA
{
  int a;
};

struct AAA* g(int x)
{
  static struct AAA aa;
  aa.a = x;
  f(x);
  return &aa;
}

/* { dg-final { scan-tree-dump "struct AAA\* klpp_g\(int x\)" } } */
/* { dg-final { scan-tree-dump "f\(x\);" } } */
