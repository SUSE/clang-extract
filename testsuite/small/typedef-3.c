/* { dg-options "-DCE_EXTRACT_FUNCTIONS=Get_X -DCE_NO_EXTERNALIZATION" }*/
typedef unsigned int __uint32_t;
typedef __uint32_t uint32_t;

struct Point {
  uint32_t x;
  uint32_t y;
};

uint32_t Get_X(struct Point *p)
{
  return p->x;
}

/* { dg-final { scan-tree-dump "typedef unsigned int __uint32_t;" } } */
/* { dg-final { scan-tree-dump "typedef __uint32_t uint32_t;" } } */
/* { dg-final { scan-tree-dump "struct Point {\n *uint32_t x;\n *uint32_t y;\n};" } } */
/* { dg-final { scan-tree-dump "uint32_t Get_X" } } */
