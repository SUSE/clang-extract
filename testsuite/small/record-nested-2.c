/* { dg-options "-DCE_EXTRACT_FUNCTIONS=Get_X -DCE_NO_EXTERNALIZATION" }*/
struct Z {
  int z;
};

typedef struct Z Z;

struct Point
{
  int x, y;
  Z *z;
};

int Get_X(struct Point *p)
{
  return p->x;
}

/* { dg-final { scan-tree-dump "struct Z {\n *int z;\n};" } } */
/* { dg-final { scan-tree-dump "typedef struct Z Z;" } } */
/* { dg-final { scan-tree-dump "struct Point\n{\n *int x, y;\n *Z \*z;\n};" } } */
/* { dg-final { scan-tree-dump "int Get_X" } } */
