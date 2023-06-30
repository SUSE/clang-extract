/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f -DCE_NO_EXTERNALIZATION" }*/

typedef struct {
  int x;
} X;

typedef struct {
  X x;
  int y;
} Y;

int f(Y *y)
{
  return y->x.x;
}

/* { dg-final { scan-tree-dump "typedef struct {\n *int x;\n} X;" } } */
/* { dg-final { scan-tree-dump "typedef struct {\n *X x;\n *int y;\n} Y;" } } */
/* { dg-final { scan-tree-dump "int f" } } */
