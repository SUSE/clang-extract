/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f -DCE_NO_EXTERNALIZATION" }*/

struct ll;

int get(struct ll *);

struct ll {
  union {
    unsigned long key;
    void *key_ptr;
  };
  struct ll *next;
};

int f(struct ll *l)
{
  return get(l);
}

/* { dg-final { scan-tree-dump-not "struct ll *{" } } */
