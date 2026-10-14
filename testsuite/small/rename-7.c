/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f -DCE_EXPORT_SYMBOLS=bbb" }*/

#define A bbb
#define B bbb

int bbb;

int f()
{
  return A + B;
}

/* { dg-final { scan-tree-dump "#define A \(\*klpe_bbb\)" } } */
/* { dg-final { scan-tree-dump "#define B \(\*klpe_bbb\)" } } */
