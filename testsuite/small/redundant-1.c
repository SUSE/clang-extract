/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f -DCE_NO_EXTERNALIZATION" }*/

struct ll;

struct ll;

int get(struct ll *);

struct ll;

int f(struct ll *l)
{
  return get(l);
}

/* { dg-final { scan-tree-dump-not "struct ll;\n+int get\(struct ll \*\);" } } */
