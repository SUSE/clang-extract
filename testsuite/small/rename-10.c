/* { dg-options "-DCE_EXTRACT_FUNCTIONS=g" }*/
#define MACRO int

MACRO f();

int g()
{
  return f();
}

/* { dg-final { scan-tree-dump "static int \(\*klp_f\)\(\);" } } */
/* { dg-final { scan-tree-dump "return klp_f\(\);" } } */
