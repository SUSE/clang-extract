/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f -DCE_NO_EXTERNALIZATION" }*/

extern "C" {
  void *memcpy(void *a, void *b, unsigned long n);
  int gVar;
};

void *f(void *a, void *b, unsigned long n)
{
  return memcpy(a, b, n);
}

/* { dg-final { scan-tree-dump "void \*memcpy" } } */
/* { dg-final { scan-tree-dump-not "int gVar;" } } */
