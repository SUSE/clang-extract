/* { dg-options "-fno-builtin-memcpy -DCE_EXTRACT_FUNCTIONS=memcpy -DCE_IPACLONES_PATH=$test_dir/glibc_private.c.000i.ipa-clones" }*/

void *memcpy(void *dst, const void *src, unsigned long n) __attribute__((always_inline));

void *memcpy(void *, const void *, unsigned long) asm("__GI_memcpy") __attribute__((visibility("hidden")));

void *memcpy(void *dst, const void *src, unsigned long n)
{
  unsigned char *c_dst = (unsigned char *) dst;
  const unsigned char *c_src = (const unsigned char *) src;

  for (unsigned long i; i < n; i++) {
    c_dst[i] = c_src[i];
  }

  return dst;
}

void f(int *a, int *b, unsigned n)
{
  memcpy(a, b, 4*sizeof(int));
}

/* { dg-final { scan-tree-dump "__GI_memcpy" } } */
/* { dg-final { scan-tree-dump "f\(int" } } */
