/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f -DCE_NO_EXTERNALIZATION" }*/
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define ONE 1
#define X MAX(ONE, 0)
#define u 8

int f(void)
{
  return X;
}

#define X 2

int g(void)
{
  return X;
}

/* { dg-final { scan-tree-dump "#define MAX\(a, b\) \(\(a\) > \(b\)" } } */
/* { dg-final { scan-tree-dump "#define ONE 1" } } */
/* { dg-final { scan-tree-dump "#define X MAX\(ONE, 0\)" } } */
/* { dg-final { scan-tree-dump "int f" } } */
/* { dg-final { scan-tree-dump-not "#define X 2" } } */
/* { dg-final { scan-tree-dump-not "#define u 8" } } */
