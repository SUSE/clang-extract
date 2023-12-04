/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f -DCE_NO_EXTERNALIZATION" }*/
struct Point
{
  int x, y;
};

int f(struct Point *p)
{
  return (*p).x;
}
/* { dg-final { scan-tree-dump "struct Point\n{\n *int x, y;\n};" } } */
/* { dg-final { scan-tree-dump "return \(\*p\).x" } } */
