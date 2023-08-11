/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f -DCE_NO_EXTERNALIZATION" }*/

typedef struct {
  int x;
  int y;
} Head;

unsigned long f()
{
  return sizeof(Head);
}

/* { dg-final { scan-tree-dump "sizeof\(Head\);" } } */
/* { dg-final { scan-tree-dump "} Head;" } } */
