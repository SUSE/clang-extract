/* { dg-options "-DCE_EXTRACT_FUNCTIONS=fts3PrefixParameter -DCE_NO_EXTERNALIZATION" }*/

struct Fts3Table {
  struct Fts3Index {
    int nPrefix;
  } *aIndex;
};

int fts3PrefixParameter(
  const char *zParam,
  int *pnIndex,
  struct Fts3Index **apIndex
){
  int a = sizeof(struct Fts3Index);
  return 0;
}

/* { dg-final { scan-tree-dump "struct Fts3Table *{" } } */
/* { dg-final { scan-tree-dump "} *\*aIndex;" } } */
