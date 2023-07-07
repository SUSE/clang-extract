/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f -DCE_NO_EXTERNALIZATION" }*/

typedef struct {
  int x;
  int y;
} Head;

unsigned long f()
{
    return __builtin_offsetof(Head, y);
}

/* { dg-final { scan-tree-dump "__builtin_offsetof" } } */
/* { dg-final { scan-tree-dump "} Head;" } } */
