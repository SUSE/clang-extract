/* { dg-options "-Wno-everything -DCE_EXTRACT_FUNCTIONS=f -DCE_NO_EXTERNALIZATION" }*/

typedef long long __m64 __attribute__((__vector_size__(8), __aligned__(8)));

typedef int __v2si __attribute__((__vector_size__(8)));

__v2si f(void)
{
  __v2si _0 = {0, 0};
  return _0;
}

/* { dg-final { scan-tree-dump "typedef int __v2si __attribute__\(\(__vector_size__\(8\)\)\);" } } */
