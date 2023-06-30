/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f -DCE_NO_EXTERNALIZATION" }*/
#define __inline__ inline

static __inline__ int func(int x)
{
  return x + 3;
}

int f(int x)
{
  return func(x);
}

/* { dg-final { scan-tree-dump "#define __inline__ inline" } } */
/* { dg-final { scan-tree-dump "static __inline__ int func\(int x\)" } } */
