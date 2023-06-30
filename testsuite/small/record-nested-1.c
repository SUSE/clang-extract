/* { dg-options "-DCE_EXTRACT_FUNCTIONS=Get_X -DCE_NO_EXTERNALIZATION" }*/

struct Z {
  int z;
};

struct Point
{
  int x, y;
  struct Z z;
};

int Get_X(struct Point *p)
{
  return p->x;
}

int getX(struct Point p)
{
  return p.x;
}

/* { dg-final { scan-tree-dump "struct Z {\n *int z;\n};" } } */
/* { dg-final { scan-tree-dump "struct Point\n{\n *int x, y;\n *struct Z z;\n};" } } */
/* { dg-final { scan-tree-dump "int Get_X" } } */
/* { dg-final { scan-tree-dump-not "int getX" } } */
