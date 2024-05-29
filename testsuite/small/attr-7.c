/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f -DCE_NO_EXTERNALIZATION" }*/
int a __attribute__((unused));

#define __aligned(x) __attribute__((__aligned__(x)))

struct S
{
  int a;
} __aligned(32);

int f(struct S *s)
{
  return s->a;
}

/* { dg-final { scan-tree-dump "} __aligned\(32\);" } } */
