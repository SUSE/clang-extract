/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f -DCE_NO_EXTERNALIZATION" }*/
// We don't need the full definition of X, yet it is bringing it to us.
/* { dg-xfail } */

typedef struct {
  int x;
} X;

typedef struct {
  X *x;
  int y;
} Y;

int f(Y *y)
{
  return y->y;
}

/* { dg-final { scan-tree-dump "typedef struct {\n *X \*x;\n *int y;\n} Y;" } } */
/* { dg-final { scan-tree-dump-not "typedef struct {\n *int x;\n} X;" } } */
/* { dg-final { scan-tree-dump "return y->y;" } } */
