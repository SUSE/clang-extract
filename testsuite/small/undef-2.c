/* { dg-options "-DCE_EXTRACT_FUNCTIONS=g -DCE_NO_EXTERNALIZATION" }*/
int x;
int yy;

#define yy x

int f(void) {
  return yy;
}

#undef yy

int g(void) {
  return yy;
}

/* { dg-final { scan-tree-dump "int yy;" } } */
/* { dg-final { scan-tree-dump "int g" } } */
/* { dg-final { scan-tree-dump-not "int f" } } */
/* { dg-final { scan-tree-dump-not "int x;" } } */
