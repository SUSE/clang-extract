/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f -DCE_EXPORT_SYMBOLS=bbb" }*/
/* { dg-xfail }*/
#define A bbb
#define B bbb

int bbb;

int f()
{
  return A + B;
}

/* { dg-final { scan-tree-dump "#define A \(\*klp_bbb\)" } } */
/* { dg-final { scan-tree-dump "#define B \(\*klp_bbb\)" } } */
