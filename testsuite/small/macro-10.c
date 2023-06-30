/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f -DCE_NO_EXTERNALIZATION" }*/
int a_symbol;

#define thismacro a_symbol

#define concat(a, b) a##b

int f()
{
  return concat(this, macro);
}

/* { dg-final { scan-tree-dump "int a_symbol;" } } */
/* { dg-final { scan-tree-dump "#define thismacro a_symbol" } } */
/* { dg-final { scan-tree-dump "#define concat\(a, b\)" } } */
