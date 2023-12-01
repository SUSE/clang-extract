/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f -DCE_NO_EXTERNALIZATION" }*/

enum {
  CONSTANT = 1,
};

#define MACRO_CONST CONSTANT

int f(void)
{
  return MACRO_CONST;
}

/* { dg-final { scan-tree-dump "#define MACRO_CONST CONSTANT" } } */
/* { dg-final { scan-tree-dump "CONSTANT = 1" } } */
/* { dg-final { scan-tree-dump "return MACRO_CONST" } } */
