/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f -DCE_NO_EXTERNALIZATION" }*/

struct Head {
  int x;
  int y;
};

unsigned long f()
{
    return __builtin_offsetof(struct Head, y);
}

/* { dg-final { scan-tree-dump "__builtin_offsetof" } } */
/* { dg-final { scan-tree-dump "struct Head {" } } */
