/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f,g" }*/
static int a;

void f(int x)
{
  a = x;
}

int g()
{
  return a;
}

/* { dg-final { scan-tree-dump "static int \*klp_a;" } } */
/* { dg-final { scan-tree-dump "\(\*klp_a\) = x;" } } */
/* { dg-final { scan-tree-dump "return \(\*klp_a\);" } } */
