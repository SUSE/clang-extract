/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f -DCE_NO_EXTERNALIZATION" }*/
int a_symbol;

#define this_macro a_symbol
#define concat(a, b) this_##b

#define macro concat(this, macro)

int f()
{
  return macro;
}

/* { dg-final { scan-tree-dump "int a_symbol;" } } */
/* { dg-final { scan-tree-dump "#define this_macro a_symbol" } } */
/* { dg-final { scan-tree-dump "#define concat\(a, b\)" } } */
/* { dg-final { scan-tree-dump "#define macro concat\(this, macro\)" } } */
