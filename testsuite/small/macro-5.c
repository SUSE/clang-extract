/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f -DCE_NO_EXTERNALIZATION" }*/
#define MAX(a, b) ((a) > (b) ? (a) : (b))

/* This macro should not be included in the final output, as it does not impact
   the final result.  */
#define a 8

int f(void)
{
  return MAX(3, 2);
}

/* { dg-final { scan-tree-dump "#define MAX\(a, b\) \(\(a\) > \(b\)" } } */
/* { dg-final { scan-tree-dump "return MAX\(3, 2\);" } } */
/* { dg-final { scan-tree-dump-not "#define a 8" } } */
