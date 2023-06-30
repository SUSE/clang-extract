/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f -DCE_NO_EXTERNALIZATION" }*/
static int f();

int f() __attribute__((unused)) {
  return 3;
}

/* { dg-final { scan-tree-dump "static int f\(\);" } } */
/* { dg-final { scan-tree-dump "int f\(\) __attribute__\(\(unused\)\)" } } */
