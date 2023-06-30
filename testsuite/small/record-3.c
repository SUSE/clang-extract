/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f -DCE_NO_EXTERNALIZATION" }*/
struct list {
  struct list *next;
};

struct list *f(const struct list *l)
{
  return l->next;
}

/* { dg-final { scan-tree-dump "struct list {\n *struct list \*next;\n};" } } */
/* { dg-final { scan-tree-dump "return l->next" } } */
