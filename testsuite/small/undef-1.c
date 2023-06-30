/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f -DCE_NO_EXTERNALIZATION" }*/
int x;
int yy;

#define yy x

int f(void) {
  return yy;
}

#undef yy

int g(void) {
  return f() + yy;
}

/* { dg-final { scan-tree-dump "int x;" } } */
/* { dg-final { scan-tree-dump "int f" } } */
/* { dg-final { scan-tree-dump-not "int g" } } */
/* { dg-final { scan-tree-dump-not "int yy;" } } */
