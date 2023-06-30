/* { dg-options "-DCE_EXTRACT_FUNCTIONS=g -DCE_NO_EXTERNALIZATION" }*/
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define ONE 1
#define X MAX(ONE, 0)

int f(void)
{
  return X;
}

#define X 2

int g(void)
{
  return X;
}

/* { dg-final { scan-tree-dump "#define X 2" } } */
/* { dg-final { scan-tree-dump "int g" } } */
/* { dg-final { scan-tree-dump-not "#define X MAX\(ONE, 0\)" } } */
