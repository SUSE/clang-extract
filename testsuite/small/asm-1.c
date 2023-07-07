/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f -DCE_NO_EXTERNALIZATION" }*/

typedef struct {
  int x;
  int y;
} Head;

unsigned long f()
{
    unsigned long dest;
    asm volatile ("mov %1, %0\n\t": "=r"(dest) : "i" (__builtin_offsetof(Head, y))
                                   : );
    return dest;
}

/* { dg-final { scan-tree-dump "asm volatile" } } */
/* { dg-final { scan-tree-dump "} Head;" } } */
