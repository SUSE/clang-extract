/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f -DCE_NO_EXTERNALIZATION" }*/
/* { dg-xfail } */

typedef long unsigned int size_t;
typedef void
  *(*AcquireMemoryHandler)(size_t),
  (*DestroyMemoryHandler)(void *),
  *(*ResizeMemoryHandler)(void *,size_t),
  *(*AcquireAlignedMemoryHandler)(const size_t,const size_t),
  (*RelinquishAlignedMemoryHandler)(void *);

extern void
  GetMagickMemoryMethods(AcquireMemoryHandler *,ResizeMemoryHandler *, DestroyMemoryHandler *),
  *ResetMagickMemory(void *,int,const size_t);

void f()
{
  (void) ResetMagickMemory((void*)0,0,0);
}

/* { dg-final { scan-tree-dump "\*\(\*AcquireMemoryHandler\)\(size_t\)," } } */
/* { dg-final { scan-tree-dump "\*\(\*DestroyMemoryHandler\)\(void *\*\)," } } */
/* { dg-final { scan-tree-dump "\*\(\*ResizeMemoryHandler\)\(void *\*, *size_t\)," } } */
