/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f -DCE_NO_EXTERNALIZATION" }*/
struct Point;

struct Point
{
  int x, y;
};

void *f(struct Point *p)
{
  return (void *)p;
}
/* { dg-final { scan-tree-dump "struct Point;" } } */
/* { dg-final { scan-tree-dump-not "struct Point\n{\n *int x, y;\n};" } } */
/* { dg-final { scan-tree-dump "return .*p;" } } */
