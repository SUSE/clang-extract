/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f -DCE_NO_EXTERNALIZATION" }*/

typedef struct {
  int x;
} A;

typedef struct {
  A a;
  int y;
} B;

int f()
{
  return __builtin_offsetof(B, y);
}

/* { dg-final { scan-tree-dump "typedef struct {\n *int x;\n} A;" } } */
/* { dg-final { scan-tree-dump "typedef struct {\n *A a;\n *int y;\n} B;" } } */
/* { dg-final { scan-tree-dump "int f" } } */
