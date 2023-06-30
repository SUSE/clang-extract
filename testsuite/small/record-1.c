/* { dg-options "-DCE_EXTRACT_FUNCTIONS=Get_X -DCE_NO_EXTERNALIZATION" }*/
struct Point;

struct Point
{
  int x, y;
};

int Get_X(struct Point *p)
{
  return p->x;
}

int getX(struct Point p)
{
  return p.x;
}

/* { dg-final { scan-tree-dump "struct Point\n{\n *int x, y;\n};" } } */
/* { dg-final { scan-tree-dump "int Get_X" } } */
/* { dg-final { scan-tree-dump-not "int getX" } } */
