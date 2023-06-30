/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f -DCE_NO_EXTERNALIZATION" }*/

typedef enum {
  LEFT,
  RIGHT
} Hand;

struct Player {
  Hand mainhand;
};

int f(struct Player *p)
{
  return p->mainhand;
}

/* { dg-final { scan-tree-dump "typedef enum {\n *LEFT,\n *RIGHT\n} Hand;" } } */
/* { dg-final { scan-tree-dump "struct Player {\n *Hand mainhand;\n};" } } */
/* { dg-final { scan-tree-dump "int f" } } */
