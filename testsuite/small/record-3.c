struct list {
  struct list *next;
};

struct list *f(const struct list *l)
{
  return l->next;
}
