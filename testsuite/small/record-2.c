/* { dg-options "-DCE_EXTRACT_FUNCTIONS=Get_X -DCE_NO_EXTERNALIZATION" }*/
struct Point
{
  int x, y;
};

typedef struct Point point_t;

int Get_X(point_t *p)
{
  return p->x;
}

int getX(struct Point p)
{
  return p.x;
}

/* { dg-final { scan-tree-dump "struct Point\n{\n *int x, y;\n};" } } */
/* { dg-final { scan-tree-dump "typedef struct Point point_t;" } } */
/* { dg-final { scan-tree-dump-not "int getX" } } */
