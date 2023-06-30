/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f -DCE_NO_EXTERNALIZATION" }*/
#define __stringify_1(x...) #x
#define __stringify(x...) __stringify_1(x)

const char *f(void)
{
  return __stringify(1);
}

/* { dg-final { scan-tree-dump "#define __stringify_1\(x...\) #x" } } */
/* { dg-final { scan-tree-dump "#define __stringify\(x...\) __stringify_1\(x\)" } } */
/* { dg-final { scan-tree-dump "return __stringify\(1\);" } } */
